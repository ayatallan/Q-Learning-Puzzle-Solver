#include <iostream>
#include <vector>
#include <cstdlib> 
#include <cmath>   
#include <ctime>  
using namespace std;

const int MAX_ROWS = 7;
const int MAX_COLS = 7;

const int EMPTY = 0;
const int WALL = 1;
const int BOX = 2;
const int STORAGE = 3;
const int BOX_IN_STORAGE = 4;
const int PLAYER = 5;
const int PLAYER_IN_STORAGE = 6;

struct State {
    int board[MAX_ROWS][MAX_COLS];
    int playerRow, playerCol;
    int box1r, box1c, box2r, box2c;
};

void printBoard(const State &S) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_COLS; ++j) {
            cout << S.board[i][j] << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

bool isGoal(const State &S) {
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_COLS; ++j) {
            if (S.board[i][j] == BOX) {
                return false;
            }
        }
    }
    return true;
}

bool isValidMove(int row, int col) {
    return (row >= 0 && row < MAX_ROWS && col >= 0 && col < MAX_COLS);
}

bool isDeadlock(const State &S) {
    if ((isValidMove(S.playerRow - 1, S.playerCol) && S.board[S.playerRow - 1][S.playerCol] != WALL) ||
        (isValidMove(S.playerRow + 1, S.playerCol) && S.board[S.playerRow + 1][S.playerCol] != WALL) ||
        (isValidMove(S.playerRow, S.playerCol - 1) && S.board[S.playerRow][S.playerCol - 1] != WALL) ||
        (isValidMove(S.playerRow, S.playerCol + 1) && S.board[S.playerRow][S.playerCol + 1] != WALL)) {
        return false; // Player can move, not a deadlock
    }

    return true; // Player cannot move, deadlock
   
}

void generateChildren(const State &S, State children[], int &numChildren) {
    numChildren = 0;
    int moves[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // Up, Down, Left, Right

    for (int i = 0; i < 4; ++i) {
        int newRow = S.playerRow + moves[i][0];
        int newCol = S.playerCol + moves[i][1];

        if (isValidMove(newRow, newCol) && S.board[newRow][newCol] != WALL) {
            State child = S;

            if (child.board[child.playerRow][child.playerCol] == PLAYER_IN_STORAGE) {
                child.board[child.playerRow][child.playerCol] = STORAGE;
            } else {
                child.board[child.playerRow][child.playerCol] = EMPTY;
            }

         
            if (S.board[newRow][newCol] == EMPTY) {
                child.playerRow = newRow;
                child.playerCol = newCol;
                child.board[newRow][newCol] = PLAYER;
                children[numChildren++] = child;
            } else if (S.board[newRow][newCol] == STORAGE) {
                child.playerRow = newRow;
                child.playerCol = newCol;
                child.board[newRow][newCol] = PLAYER_IN_STORAGE;
                children[numChildren++] = child;
            } else if (S.board[newRow][newCol] == BOX || S.board[newRow][newCol] == BOX_IN_STORAGE) {
                int newbr = newRow + moves[i][0];
                int newbc = newCol + moves[i][1];
                if (isValidMove(newbr, newbc) && (S.board[newbr][newbc] == EMPTY || S.board[newbr][newbc] == STORAGE)) {
                    child.playerRow = newRow;
                    child.playerCol = newCol;
                    child.board[newRow][newCol] = PLAYER;

                    if (S.board[newbr][newbc] == EMPTY) {
                        child.board[newbr][newbc] = BOX;
                    } else {
                        child.board[newbr][newbc] = BOX_IN_STORAGE;
                    }

                    if (S.board[newRow][newCol] == BOX_IN_STORAGE) {
                        child.board[newRow][newCol] = PLAYER_IN_STORAGE;
                    }

                    children[numChildren++] = child;
                }
            }
        }
    }
}

// Define Q-learning parameters
const int num_states = 100;
const int num_actions = 4;
const double Y = 0.9;
double epsilon = 0.9;
const int num_episodes = 8000;

// Define Q-table
double Q[num_states][num_actions] = {0};

// Function to convert a state to its corresponding index in the Q-table
int stateToIndex(const State &state) {
    
    return state.playerRow * MAX_COLS + state.playerCol;
}


int chooseAction(const State &state) {
    int stateIndex = stateToIndex(state);
   if (rand() / double(RAND_MAX) < epsilon) {
       
        return rand() % num_actions;}
     else {
        
        int maxAction = 0;
        for (int a = 1; a < num_actions; ++a) {
            if (Q[stateIndex][a] > Q[stateIndex][maxAction]) {
                maxAction = a;
            }
        }
        return maxAction;
    }
}
int maxactionn(const State &state){
     int stateIndex = stateToIndex(state);
 int maxAction = 0;
        for (int a = 1; a < num_actions; ++a) {
            if (Q[stateIndex][a] > Q[stateIndex][maxAction]) {
                maxAction = a;
            }
        }
        return maxAction;
    }
// Function to update the Q-values using the Q-learning algorithm
void updateQValue(int stateIndex, int action, double reward, const State &nextState) {
    int nextStateIndex = stateToIndex(nextState);
    Q[stateIndex][action] = reward + (Y * Q[nextStateIndex][chooseAction(nextState)]);
}

// Function to calculate the reward based on the action and next state
double calculateReward(const State &currentState, const State &nextState) {
    
    if (isGoal(nextState)) {
        return 1000.0;
    }
    
    if (isDeadlock(nextState)) {
        return -100.0;
    }
    
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_COLS; ++j) {
            if (nextState.board[i][j] == BOX_IN_STORAGE) {
                return 100.0;
            }
        }
    }
    
    return -0.1;
}


State performAction(const State &currentState, int action) {
    State children[4];
    int numChildren;
    generateChildren(currentState, children, numChildren);
    if (numChildren > 0) {
        return children[action % numChildren]; 
    }
    return currentState; 
}


void evaluatePolicy(const State &initialState) {
    State currentState = initialState;
    int steps = 0;

    while (!isGoal(currentState) && steps < 1000) { 
        int action = maxactionn(currentState);
        State nextState = performAction(currentState, action);
        double reward = calculateReward(currentState, nextState);
        updateQValue(stateToIndex(currentState), action, reward, nextState);
        currentState = nextState;
        steps++;
        if (steps <= 100) { 
            cout << "Step " << steps << ":\n";
            printBoard(currentState);
        }
    }

    cout << "Goal reached in " << steps << " steps!" << endl;
}

int main() {
    srand(time(0)); 

    // Define the initial state
    State initialState = {
        {
            {1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 0, 0, 1},
            {1, 0, 0, 3, 0, 0, 1},
            {1, 0, 2, 0, 0, 0, 1},
            {1, 0, 3, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1}
        },
        1, 1, 2, 3, 4, 2
    };

    cout << "Enter the player row: ";
    cin >> initialState.playerRow;
    cout << "Enter the player column: ";
    cin >> initialState.playerCol;

    if (initialState.board[initialState.playerRow][initialState.playerCol] == STORAGE) {
        initialState.board[initialState.playerRow][initialState.playerCol] = PLAYER_IN_STORAGE;
    } else {
        initialState.board[initialState.playerRow][initialState.playerCol] = PLAYER;
    }

    printBoard(initialState);

    // Q-learning algorithm
    for (int episode = 0; episode < num_episodes; ++episode) {
        State currentState = initialState;
        int steps = 0;

        while (!isGoal(currentState) && steps < 3000) { 
            int action = chooseAction(currentState);
            State nextState = performAction(currentState, action);
            double reward = calculateReward(currentState, nextState);
            int currentStateIndex = stateToIndex(currentState);
            updateQValue(currentStateIndex, action, reward, nextState);
            currentState = nextState;
            steps++;
            
            
        }
        

      
        epsilon *= 0.99;
        
    }
    
    //for (int i = 0; i < num_states; ++i) {
        //for (int j = 0; j < num_actions; ++j) {
          //  cout << Q[i][j] << " ";
        //}
      //  cout << endl; // Newline for each state
    //}

   //cout<<"===================================="<<endl;
    evaluatePolicy(initialState);
   //cout<<"===================================="<<endl;
   //for (int i = 0; i < num_states; ++i) {
        //for (int j = 0; j < num_actions; ++j) {
          //  cout << Q[i][j] << " ";
        //}
      //  cout << endl; // Newline for each state
    //}
 //cout<<"=================================="<<endl;
 //evaluatePolicy(initialState);
 //cout<<"=================================="<<endl;
 //evaluatePolicy(initialState);
  //cout<<"=================================="<<endl;
 //evaluatePolicy(initialState);
 cout<<"====<zainah and ayat>===";
    return 0;
    
}

