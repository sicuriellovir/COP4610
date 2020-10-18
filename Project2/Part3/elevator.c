#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/mutex.h>

//max length of procf read message
#define BUF_LEN 1000

extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int, int, int);
extern long (*STUB_stop_elevator)(void);

//this will allow us to interact with the running thread for the scheduler
struct task_struct* schedulerThread = NULL;
//defines the different states of the elevator
enum elevatorState { OFFLINE, IDLE, LOADING, UP, DOWN };

//the mutex lock used for shared data
struct mutex lock;

struct Passenger {
//allows us to create a linked list of passengers. There should be one of
//these lists for each floor and one for the elevator
	struct list_head passList;

//this will be 0 if the passenger is not infected or 1 if they are
	int infected;

//the floor that the passenger wants to go to
	int destinationFloor;
};

struct Elevator {
//the elevator's current state
	enum elevatorState state;

//number of passengers in the elevator
	int numOfPassengers;

//the elevator's status (0 if there are no infected passengers, 1 otherwise)
	int status;

//the floor that the elevator is currently on
	int floor;

//the total number of passengers serviced by the elevator
	int numServiced;

/* indicates whether the elevator is in the process of deactivating. It will
be 0 if the elevator is not deactivating or 1 otherwise */
	int deactivating;

//number of passengers waiting to board the elevator
	int numWaiting;
} elevator;

//prototype for procfile_read
static ssize_t procfile_read(struct file* file, char* ubuf, size_t count, loff_t* ppos);

//creates the struct needed for procf operations
static struct file_operations procfile_fops = {
	.owner = THIS_MODULE,
	.read = procfile_read,
};

//pointer to the proc entry
static struct proc_dir_entry* proc_entry;

//buffer to store the procf read message
char* msg;

//variable to hold the length of the procf read message
static int procfs_buf_len;

char statusTemplate[] = "Elevator state: %s\nElevator status: %s\nCurrent floor: %d\nNumber of passengers: %d\nNumber of passengers waiting: %d\nNumber passengers serviced: %d\n\n\n";
char floorTemplate[] = "[%c] Floor %d:\t%d\t%s\n";

//declares an array of passenger lists (one for each floor) and a passenger list
//for the elevator
struct Passenger floorPassengerLists[10];
struct Passenger elevatorPassengerList;

//function prototypes
int mainLoop(void* arg);
int goUp(void);
int goDown(void);
int getFloorPassengerNum(int floor);
char* getFloorPassengerStr(int floor);
void loadPassengersIntoElevator(void);
void unloadPassengersFromElevator(void);
void infectElevator(void);
void deallocatePassList(struct Passenger* pList);

MODULE_LICENSE("GPL");

//implementation of the start_elevator syscall
long start_elevator(void)
{
	if (elevator.state == OFFLINE)
	{
		if(mutex_lock_interruptible(&lock) == 0)
		{
			elevator.state = IDLE;
			schedulerThread = kthread_run(mainLoop, NULL, "ElevatorScheduler");
      mutex_unlock(&lock);
			return 0;
		}
	}
	return 1;
}

//implementation of the issue_request syscall
long issue_request(int start_floor, int destination_floor, int type)
{
//checks that the parameters are valid
	if (start_floor > 10 || start_floor < 1 || destination_floor > 10 ||
		destination_floor < 1 || (type != 0 && type != 1))
		return 1;

//creates the passenger and adds it to the appropriate floor's passenger list
//returns without making changes if memory allocation failed
	struct Passenger* pass = kmalloc(sizeof(struct Passenger), __GFP_RECLAIM);
	if (pass == NULL)
		return -ENOMEM;
	pass->infected = type;
	pass->destinationFloor = destination_floor;
  if(mutex_lock_interruptible(&lock) == 0)
  {
	  list_add_tail(&pass->passList, &floorPassengerLists[start_floor - 1].passList);
    elevator.numWaiting += 1;
    mutex_unlock(&lock);
  }

	return 0;
}

//implementation of the stop_elevator syscall
long stop_elevator(void)
{
  if(mutex_lock_interruptible(&lock) == 0)
  {
	   if (elevator.deactivating == 1 || elevator.state == OFFLINE)
     {
       mutex_unlock(&lock);
		   return 1;
     }
	   else
	   {
		   elevator.deactivating = 1;
       mutex_unlock(&lock);
			 kthread_stop(schedulerThread);
//the code below won't run until after the thread returns
//(happens when the elevator unloads all passengers)
       if(mutex_lock_interruptible(&lock) == 0)
       {
  			 elevator.state = OFFLINE;
  			 elevator.deactivating = 0;
  			 elevator.status = 0;
         mutex_unlock(&lock);
  			 return 0;
       }
	   }
   }
}

/*this is the main loop that will control the elevator's behavior
it will run in its own thread and service passengers
Note: this function doesn't need to return an int and the parameter
is pointless, but kthread_run throws errors if it's not defined this way*/
int mainLoop(void* arg)
{
	while (1)
	{
    if(mutex_lock_interruptible(&lock) == 0)
    {
  		if (!(elevator.numOfPassengers == 0 && elevator.numWaiting == 0))
  		{
  			elevator.state = UP;
        mutex_unlock(&lock);
  			loadPassengersIntoElevator();
  			while (elevator.state != IDLE && goUp() == 1)
  			{
  				unloadPassengersFromElevator();
  				loadPassengersIntoElevator();

  //labels the elevator as deactivating if the thread should stop
  //(happens when stop_elevator is called)
          if(mutex_lock_interruptible(&lock) == 0)
          {
    				if (kthread_should_stop() && elevator.deactivating == 0)
    					elevator.deactivating = 1;

    //returns from the thread if the elevator has finished deactivating
    				if (elevator.deactivating == 1 && elevator.numOfPassengers == 0)
            {
              mutex_unlock(&lock);
    					return 1;
            }

    				if (elevator.numWaiting == 0 && elevator.numOfPassengers == 0)
    					elevator.state = IDLE;
            mutex_unlock(&lock);
          }
  			}

        if(mutex_lock_interruptible(&lock) == 0)
        {
    			if (elevator.state != IDLE)
    			{
    				elevator.state = DOWN;
            mutex_unlock(&lock);
    				loadPassengersIntoElevator();
    			}
          else
            mutex_unlock(&lock);
        }

  			while (elevator.state != IDLE && goDown() == 1)
  			{
  				unloadPassengersFromElevator();
  				loadPassengersIntoElevator();

  //labels the elevator as deactivating if the thread should stop
  //(happens when stop_elevator is called)
          if(mutex_lock_interruptible(&lock) == 0)
          {
    				if (kthread_should_stop() && elevator.deactivating == 0)
    					elevator.deactivating = 1;

  //returns from the thread if the elevator has finished deactivating
    				if (elevator.deactivating == 1 && elevator.numOfPassengers == 0)
            {
              mutex_unlock(&lock);
    					return 1;
            }

    				if (elevator.numWaiting == 0 && elevator.numOfPassengers == 0)
    					elevator.state = IDLE;
            mutex_unlock(&lock);
          }
        }
			}
			else
				mutex_unlock(&lock);
		}

    if(mutex_lock_interruptible(&lock) == 0)
    {
  		if (elevator.deactivating == 1 && elevator.numOfPassengers == 0)
      {
        mutex_unlock(&lock);
  			return 1;
      }
      mutex_unlock(&lock);
    }
	}
}

//changes elevator's state to UP and increments floor if possible. Returns 1 if
//floor was incremented successfully or 0 if the elevator is on the top floor
int goUp(void)
{
  if(mutex_lock_interruptible(&lock) == 0)
  {
  	elevator.state = UP;
  	if (elevator.floor != 10)
  	{
			mutex_unlock(&lock);
  		ssleep(2);
			mutex_lock_interruptible(&lock);
  		elevator.floor++;
      mutex_unlock(&lock);
  		return 1;
  	}
    mutex_unlock(&lock);
  	return 0;
  }
}

//changes elevator's state to DOWN and decrements floor if possible. Returns 1 if
//floor was decremented successfully or 0 if the elevator is on the bottom floor
int goDown(void)
{
  if(mutex_lock_interruptible(&lock) == 0)
  {
  	elevator.state = DOWN;
  	if (elevator.floor != 1)
  	{
			mutex_unlock(&lock);
  		ssleep(2);
			mutex_lock_interruptible(&lock);
  		elevator.floor--;
      mutex_unlock(&lock);
  		return 1;
  	}
    mutex_unlock(&lock);
  	return 0;
  }
}

//returns the number of passenger on a floor
int getFloorPassengerNum(int floor)
{
	int count = 0;
	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

  if(mutex_lock_interruptible(&lock) == 0)
  {
  	list_for_each_safe(temp, dummy, &floorPassengerLists[floor - 1].passList)
  	{
  		pass = list_entry(temp, struct Passenger, passList);
  		count++;
  	}
    mutex_unlock(&lock);
  }

	return count;
}

//returns the string that will be printed to represent passengers on a specified
//floor during proc read
char* getFloorPassengerStr(int floor)
{
	char* str = kmalloc(1 * sizeof(char), __GFP_RECLAIM);
	*str = '\0';
	char* tempStr = NULL;
	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

  if(mutex_lock_interruptible(&lock) == 0)
  {
  	list_for_each_safe(temp, dummy, &floorPassengerLists[floor - 1].passList)
  	{
  		tempStr = str;
  		str = kmalloc((strlen(str) + 3) * sizeof(char), __GFP_RECLAIM);
  		strcpy(str, tempStr);
  		kfree(tempStr);
  		pass = list_entry(temp, struct Passenger, passList);
  		if (pass->infected == 0)
  			strcat(str, "| ");
  		else
  			strcat(str, "X ");
  	}
    mutex_unlock(&lock);
  }

	return str;
}

//deallocates a passenger list
void deallocatePassList(struct Passenger* pList)
{
	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

  if(mutex_lock_interruptible(&lock) == 0)
  {
  	list_for_each_safe(temp, dummy, &pList->passList)
  	{
  		pass = list_entry(temp, struct Passenger, passList);
  		list_del(temp);
  		kfree(pass);
  	}
    mutex_unlock(&lock);
  }
}

//loads passengers into the elevator if possible at the current floor
void loadPassengersIntoElevator(void)
{
	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

//saves the elevator's state before loading and sets current state to loading
  if(mutex_lock_interruptible(&lock) == 0)
  {
  	enum elevatorState prevState = elevator.state;
  	elevator.state = LOADING;

		list_for_each_safe(temp, dummy, &floorPassengerLists[elevator.floor - 1].passList)
		{
			if (elevator.numOfPassengers == 10 || elevator.deactivating == 1)
      {
        mutex_unlock(&lock);
				return;
      }
			pass = list_entry(temp, struct Passenger, passList);

	/*this extremely long if condition verifies that the passenger is going
	in the same direction as the elevator and an uninfected passenger isn't
	going into an infected elevator*/
			if (((prevState == UP && pass->destinationFloor >= elevator.floor)
				|| (prevState == DOWN && pass->destinationFloor <= elevator.floor))
				&& !(pass->infected == 0 && elevator.status == 1))
			{
	//sleeps for a second then loads the passenger into the elevator
				mutex_unlock(&lock);
				ssleep(1);
				mutex_lock_interruptible(&lock);
				list_move_tail(&pass->passList, &elevatorPassengerList.passList);
				elevator.numOfPassengers++;
				elevator.numWaiting--;

	//if an infected passenger is entering an uninfected elevator, infect all
	//passengers in the elevator
				if (pass->infected == 1 && elevator.status == 0)
        {
          mutex_unlock(&lock);
					infectElevator();
          mutex_lock_interruptible(&lock);
        }
			}
		}
    //resets elevator's state to whatever it was before loading
    elevator.state = prevState;
    mutex_unlock(&lock);
	}
}

//unloads passengers from the elevator if possible at the current floor
void unloadPassengersFromElevator(void)
{
	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

//saves the elevator's state before loading and sets current state to loading
  if(mutex_lock_interruptible(&lock) == 0)
  {
  	enum elevatorState prevState = elevator.state;
  	elevator.state = LOADING;
  	list_for_each_safe(temp, dummy, &elevatorPassengerList.passList)
  	{
  		pass = list_entry(temp, struct Passenger, passList);
  		if (pass->destinationFloor == elevator.floor)
  		{
  //sleeps for a second then unloads the passenger from the elevator
				mutex_unlock(&lock);
				ssleep(1);
				mutex_lock_interruptible(&lock);
  			list_del(temp);
  			kfree(pass);
  			elevator.numOfPassengers--;
  			elevator.numServiced++;
  		}
  	}
  //labels the elevator as uninfected if the elevator is empty
  	if (elevator.numOfPassengers == 0)
  		elevator.status = 0;

  //resets elevator's state to whatever it was before loading
  	elevator.state = prevState;
    mutex_unlock(&lock);
  }
}

//infects all passengers in the elevator and labels the elevator as infected
void infectElevator(void)
{
	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

  if(mutex_lock_interruptible(&lock) == 0)
  {
  	elevator.status = 1;
  	list_for_each_safe(temp, dummy, &elevatorPassengerList.passList)
  	{
  		pass = list_entry(temp, struct Passenger, passList);
  		pass->infected = 1;
  	}
    mutex_unlock(&lock);
  }
}

static ssize_t procfile_read(struct file* file, char* ubuf, size_t count, loff_t* ppos)
{
	char* state = kmalloc(8 * sizeof(char), __GFP_RECLAIM);
	char* status = kmalloc(11 * sizeof(char), __GFP_RECLAIM);
	char* temp = kmalloc(BUF_LEN * sizeof(char), __GFP_RECLAIM);

//determines the text used for the elevator's state
  if(mutex_lock_interruptible(&lock) == 0)
  {
  	if (elevator.state == OFFLINE)
  		strcpy(state, "OFFLINE");
  	else if (elevator.state == IDLE)
  		strcpy(state, "IDLE");
  	else if (elevator.state == LOADING)
  		strcpy(state, "LOADING");
  	else if (elevator.state == UP)
  		strcpy(state, "UP");
  	else if (elevator.state == DOWN)
  		strcpy(state, "DOWN");

  //determines the text used for the elevator's status
  	if (elevator.status == 0)
  		strcpy(status, "Uninfected");
  	else
  		strcpy(status, "Infected");

  //copies the formatted text to the msg that will be returned to the user
  	snprintf(msg, BUF_LEN, statusTemplate, state, status, elevator.floor,
  		elevator.numOfPassengers, elevator.numWaiting, elevator.numServiced);
		mutex_unlock(&lock);

  //determines the string that will be printed for each floor and concatenates
  //it to the final msg
  	int i = 10;
  	for (; i > 0; i--)
  	{
  		int numPassOnFloor = getFloorPassengerNum(i);
  		char floorIndicator = ' ';
      if(mutex_lock_interruptible(&lock) == 0)
      {
    		if (elevator.floor == i)
    			floorIndicator = '*';
        mutex_unlock(&lock);
    		snprintf(temp, BUF_LEN, floorTemplate, floorIndicator, i, numPassOnFloor,
    			getFloorPassengerStr(i));
    		strcat(msg, temp);
      }
  	}
  }

//concatenates the final line to the msg
	strcat(msg, "\n( \"|\" for human, \"X\" for zombie )\n");

	procfs_buf_len = strlen(msg);

	if (*ppos > 0 || count < procfs_buf_len)
		return 0;

//sends the final msg to the user
	if (copy_to_user(ubuf, msg, procfs_buf_len))
		return -EFAULT;

	*ppos = procfs_buf_len;

	kfree(state);
	kfree(status);
	kfree(temp);

	return procfs_buf_len;
}

//runs when module is loaded
static int elevator_init(void) {
  //initializes the mutex
  mutex_init(&lock);

//initializes the elevator
	elevator.state = OFFLINE;
	elevator.numOfPassengers = 0;
	elevator.status = 0;
	elevator.floor = 1;
	elevator.deactivating = 0;
	elevator.numWaiting = 0;

//initializes the passenger lists for the elevator and each floor
	int i = 0;
	for (; i < 10; i++)
		INIT_LIST_HEAD(&floorPassengerLists[i].passList);
	INIT_LIST_HEAD(&elevatorPassengerList.passList);

//sets the stub function pointers to their appropriate functions
  STUB_start_elevator = start_elevator;
  STUB_issue_request = issue_request;
  STUB_stop_elevator = stop_elevator;

//creates a proc entry for the elevator
	proc_entry = proc_create("elevator", 0666, NULL, &procfile_fops);

//allocates memory for the proc msg
	msg = kmalloc(BUF_LEN * sizeof(char), __GFP_RECLAIM);

	return 0;
}
module_init(elevator_init);

//runs when module is unloaded
static void elevator_exit(void) {
//resets the stub function pointers to null
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;

	struct list_head *temp;
	struct list_head *dummy;
	struct Passenger *pass;

//deallocates the passenger lists for the elevator and each floor
	int i = 0;
	for (; i < 10; i++)
		deallocatePassList(&floorPassengerLists[i]);
	deallocatePassList(&elevatorPassengerList);

//removes the proc entry
	proc_remove(proc_entry);
	mutex_destroy(&lock);
}
module_exit(elevator_exit);
