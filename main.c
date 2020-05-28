#include "snek_api.h"
#include <unistd.h>

void play_game() {
	printf("starting\n");
	//Board initialized, struct has pointer to snek
	GameBoard* board = init_board();
	show_board(board);

	int axis = AXIS_INIT;
	int direction = DIR_INIT;
	int play_on = 1;
	int coord[2];
  int moogleLocX;
  int moogleLocY;

  //Generate Hamiltonian Circuit
  int arr_count = 1;  
  int arr[BOARD_SIZE][BOARD_SIZE]; //Hamiltonian Array
  int max_ham_num; //The maximum value that will occur in the Hamiltonian Array

  if (BOARD_SIZE%2 == 0){//Even sized board
    max_ham_num = BOARD_SIZE*BOARD_SIZE;
    //First Column
    while(arr_count <= BOARD_SIZE) {
      arr[0][arr_count-1] = arr_count;
      arr_count++;
    }
    //Subsequent Columns
    for(int i = 1; i < BOARD_SIZE; i++) {
      if(i % 2 == 0) {
       for(int j = 1; j < BOARD_SIZE; j++) 
        arr[i][j] = arr_count++;
      }else {
        for(int j = BOARD_SIZE-1; j > 0; j--) 
          arr[i][j] = arr_count++;
      }
    }
    //First row
    for(int i = BOARD_SIZE-1; i > 0; i--){
      arr[i][0] = arr_count++;
    }

  }else if (BOARD_SIZE%2 == 1){//Odd sized board
    max_ham_num = BOARD_SIZE*BOARD_SIZE - 1;
    //First Column
    while(arr_count <= BOARD_SIZE) {
      arr[0][arr_count-1] = arr_count;
	    arr_count++;
    }
    //Subsequent Columns Until BOARD_SIZE-2
    for(int i = 1; i < BOARD_SIZE-2; i++) {
      if(i % 2 == 0) {
        for(int j = 1; j < BOARD_SIZE; j++) {
          arr[i][j] = arr_count++;
        }
      }else {
        for(int j = BOARD_SIZE-1; j > 0; j--) {
          arr[i][j] = arr_count++;
        }
      }
    }
    //Last Two Columns
    for (int i = BOARD_SIZE-1; i > 0; i--){
      if (i % 2 == 0){
        arr[BOARD_SIZE-2][i] = arr_count++;
        arr[BOARD_SIZE-1][i] = arr_count++;
      }else{
        if (i % 2 == 1){
          arr[BOARD_SIZE-1][i] = arr_count++;
          arr[BOARD_SIZE-2][i] = arr_count++;
        }   
      }
    }
    arr[BOARD_SIZE-1][0] = -1; //Initialize one inaccessible square to -1
    //First Row
    for(int i = BOARD_SIZE-2; i > 0; i--) {
      arr[i][0] = arr_count++;
    }
  }

  //Start Game
	while (play_on){	
    //In the case where a moogle spawns in the inaccessible square, alter the Hamiltonian Array
    if (BOARD_SIZE%2 == 1 && (board->cell_value[0][BOARD_SIZE-1] == 20 ||board->cell_value[0][BOARD_SIZE-1] == 60)){
      arr[BOARD_SIZE-1][0] = BOARD_SIZE*BOARD_SIZE - BOARD_SIZE + 1;
      arr[BOARD_SIZE-2][1] = -1;
    }else if (BOARD_SIZE%2 == 1){
      arr[BOARD_SIZE-2][1] = BOARD_SIZE*BOARD_SIZE - BOARD_SIZE + 1;
    }

		coord[x] = board->snek->head->coord[x];
		coord[y] = board->snek->head->coord[y];
		int curr = arr[coord[y]][coord[x]];
		int found = 0;
		int newX, newY;

    int tailNum = arr[board->snek -> tail -> coord[x]][board->snek -> tail -> coord[y]]; //Associating tail coordinates to its Hamiltonian Number
    int headNum = arr[board->snek -> head -> coord[x]][board->snek -> head -> coord[y]]; //Associating head coordinates to its Hamiltonian Number
    if (headNum == -1){headNum = BOARD_SIZE*BOARD_SIZE - BOARD_SIZE +1;} 
    //In the case where head is at the inaccessible square, make head take on the same value as the value before the Hamiltonian Array alteration

    int moogleNum;
    int greatest = -1;
    //Initilizing the neighbours
    int leftNum = -1;
    int rightNum = -1;
    int upNum = -1;
    int downNum = -1;

    if (MOOGLE_FLAG) { //If there is a target
      //Find Moogle's coordinates
      for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
          if(board->cell_value[i][j] > 1) {
            moogleLocX = j;
            moogleLocY = i;
          }
        }
      }
      moogleNum = arr[moogleLocX][moogleLocY]; //Find Moogle's Hamiltonian number

      int distTail = (headNum < tailNum) ? tailNum-headNum-1:tailNum-headNum-1+max_ham_num;
      //distTail calculates the number of cells that are outside of the head and the tail. distTail is the number of moves the snake takes to reach the tail by following the Hamiltonian path
      int distMoogle = (headNum < moogleNum) ? moogleNum-headNum-1:moogleNum-headNum-1+max_ham_num;
      //distMoogle is the number of moves that the snake can go to reach the Moogle by following the Hamiltonian path
      int available = distTail-3; //available is initially set to distTail - 3. 3 is a buffer.

      if(distMoogle < distTail){available--;} //This is saying if distMoogle < distTail, there is the possibility where after the snake eats a moogle, the head would collide with the tail. This happens when the Moogle is 1 cell away from the tail. As a preventative measure, subtract one from available.
      if(distMoogle < available){available = distMoogle;} //available is set to distMoogle if it is larger than distMoogle
      //Available is essentially the number of safe moves that the head of the snake can take by following the Hamiltonian path at this exact moment. It is <= min{distTail, distMoogle}. This is important to short cut finding because when a short cut is performed (a path that is not the Hamiltonian path), it is only safe to short cut as much as the number of moves the head can safely move by following a Hamiltonina path.
      if(available < 0){available = 0;} //If the available is below zero, this means that there is no possible short cut that is safe. In this case, the snake simply follows the regular Hamiltonian path

      //Getting the Hamiltonian numbers for the neighbours adjacent to the head
      if(coord[x] > 0 && board->occupancy[coord[y]][coord[x]-1] != 1)
        leftNum = arr[coord[x]-1][coord[y]];
      if(coord[x] < BOARD_SIZE-1 && board->occupancy[coord[y]][coord[x]+1] != 1)
        rightNum = arr[coord[x]+1][coord[y]];
      if(coord[y] > 0 && board->occupancy[coord[y]-1][coord[x]] != 1)
        upNum = arr[coord[x]][coord[y]-1];
      if(coord[y] < BOARD_SIZE-1 && board->occupancy[coord[y]+1][coord[x]] != 1)
        downNum = arr[coord[x]][coord[y]+1];

      int adjacent[] = {leftNum , rightNum, upNum, downNum};
      int greatestDist = -1;
      
      for(int i = 0; i < 4; i++) {
        int distance = (headNum < adjacent[i]) ? adjacent[i]-headNum-1 : adjacent[i]-headNum-1+max_ham_num;
        //distance is the distance between the head and its neighbours.
        if(adjacent[i] != -1 && distance <= available && distance > greatestDist) { //This determines the greatest distance that is also less than available. This allows the snake to select the neighbour that is the closest to the target yet it does not overshoot the target.
          greatestDist = distance;
          greatest = adjacent[i];
        }
      }

    }
    
    if(greatest == -1) //If no shortcut is found, follow normal Hamilatonian circuit path
      greatest = (headNum < max_ham_num) ? headNum + 1 : 1;

    //Converting Hamiltonian number into the corresponding game board coordinates
    for(newX = 0; newX < BOARD_SIZE && found == 0; newX += found == 0) { 
      for(newY = 0; newY < BOARD_SIZE && found == 0; newY += found == 0) {
        if(greatest == arr[newX][newY]) 
          found = 1;
      }
    } 

    //In the event the Hamiltonian circuit value is occupied, find an open neighbour
    if(board->occupancy[newY][newX] == 1) {
      newY = board->snek->head->coord[y];
      newX = board->snek->head->coord[x];
      if(newX > 0 && board->occupancy[newY][newX-1] != 1 && coord[x] != newX-1) 
        newX--;
      else if(newX < BOARD_SIZE-1 && board->occupancy[newY][newX+1] != 1 && coord[x] != newX+1) 
        newX++;
      else if(newY > 0 && board->occupancy[newY-1][newX] != 1 && coord[y] != newY-1)
        newY--;
      else if(newY < BOARD_SIZE-1 && board->occupancy[newY+1][newX] != 1 && coord[y] != newY+1) 
        newY++;
    }

    //Based on the new coordinate values, assign the next axis and direction
    if(newX > coord[x]) {
      axis = AXIS_X;
      direction = RIGHT;
    }else if(newX < coord[x]) {
      direction = LEFT;
      axis = AXIS_X;
    }else if(newY > coord[y]) {
      axis = AXIS_Y;
      direction = DOWN;
    }else if(newY < coord[y]) {
      axis = AXIS_Y;
      direction = UP;
    }

		show_board(board);
		play_on = advance_frame(axis, direction, board);

    //Printing some information
		if (axis == AXIS_X){
			if (direction == RIGHT){
				printf("RIGHT");
			} else {
				printf("LEFT");
			}
		} else {
			if (direction == UP){
				printf("UP");
			} else {
				printf("DOWN");
			}
		} printf("\n");
		usleep(15000);
	}
  
	end_game(&board);
}

int main(){
    play_game();
    
    int score = SCORE;
    int numm = MOOGLES_EATEN;

    FILE *stream = fopen("performance.txt", "a");
    fprintf(stream, "%d\n", score);
    fclose(stream);

    FILE *stream1 = fopen("moogles.txt", "a");
    fprintf(stream1, "%d\n", numm);
    fclose(stream1);

    return 0;
}
