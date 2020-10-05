#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");

extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int, int, int);
extern long (*STUB_stop_elevator)(void);
void mainLoop(void);

enum elevatorState { OFFLINE, IDLE, LOADING, UP, DOWN };

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

//declares an array of passenger lists (one for each floor) and a passenger list
//for the elevator
struct Passenger floorPassengerLists[10];
struct Passenger elevatorPassengerList;

long start_elevator(void) {
	//debugging message
	printk(KERN_NOTICE "Printing from start_elevator MODULE");

	if (elevator.state == OFFLINE)
	{
		elevator.state = IDLE;
		return 0;
	}
	return 1;
}

long issue_request(int start_floor, int destination_floor, int type) {
	//debugging message
	printk(KERN_NOTICE "Printing from issue_request MODULE");

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
	list_add_tail(&pass->passList, &floorPassengerLists[start_floor - 1].passList);

	elevator.numWaiting += 1;

	return 0;
}

long stop_elevator(void) {
	//debugging message
	printk(KERN_NOTICE "Printing from stop_elevator MODULE");

	if (elevator.deactivating == 1 || elevator.state == OFFLINE)
		return 1;
	else
	{
		elevator.deactivating = 1;
		elevator.numWaiting = 0;
		return 0;
	}
}

//this is the main loop that will control the elevator's behavior
//it will run in its own thread and service passengers
void mainLoop(void)
{
	while (1)
	{
		
	}
}

//runs when module is loaded
static int elevator_init(void) {
	//sets the stub function pointers to their appropriate functions
	STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;

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

	printk(KERN_ALERT "ELEVATOR MODULE LOADED\n");
	return 0;
}
module_init(elevator_init);

//runs when module is unloaded
static void elevator_exit(void) {
	//resets the stub function pointers to null
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;

	printk(KERN_ALERT "ELEVATOR MODULE UNLOADED\n");
}
module_exit(elevator_exit);
