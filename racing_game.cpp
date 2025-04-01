#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Function to see if you pressed the key
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    
    return 0;
}

// Function to get char before you input
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

// Function to clear the screen
void clearScreen() {
    std::cout << "\033[2J\033[1;1H";  
}

class Game {
private:
    const int width = 20;
    const int height = 15;
    const int roadWidth = 10;
    int playerPosition;
    int score;
    int speed;
    int difficulty;
    std::vector<std::vector<char>> gameBoard;
    bool gameOver;

public:
    Game() : playerPosition(width / 2), score(0), speed(100), difficulty(0), gameOver(false) {
        // Init game board
        gameBoard = std::vector<std::vector<char>>(height, std::vector<char>(width, ' '));
        
        // Init road
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (x == (width - roadWidth) / 2 || x == (width + roadWidth) / 2) {
                    gameBoard[y][x] = '|';
                }
            }
        }
        
        // Place the player
        gameBoard[height - 1][playerPosition] = 'A';
    }

    void drawBoard() {
        clearScreen();  // Clear cons
        
        std::cout << "Score: " << score << " | Speed: " << (100 - speed + 10) << "mph\n";
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                std::cout << gameBoard[y][x];
            }
            std::cout << std::endl;
        }
        
        std::cout << "Controls: A (left), D (right), Q (quit)\n";
    }

    void movePlayer(char direction) {
        // Clear current player position
        gameBoard[height - 1][playerPosition] = ' ';
        
        // Update player position
        if (direction == 'a' && playerPosition > (width - roadWidth) / 2 + 1) {
            playerPosition--;
        } else if (direction == 'd' && playerPosition < (width + roadWidth) / 2 - 1) {
            playerPosition++;
        }
        
        // Check for collision
        if (gameBoard[height - 1][playerPosition] == 'X') {
            gameOver = true;
            return;
        }
        
        // Place player new positiomn
        gameBoard[height - 1][playerPosition] = 'A';
    }

    void updateObstacles() {
        // Move obstacles down
        for (int y = height - 1; y > 0; y--) {
            for (int x = (width - roadWidth) / 2 + 1; x < (width + roadWidth) / 2; x++) {
                if (gameBoard[y - 1][x] == 'X' && y == height - 1 && x == playerPosition) {
                    // Collision detection when moving obstacles down
                    gameOver = true;
                    return;
                }
                gameBoard[y][x] = gameBoard[y - 1][x];
            }
        }
        
        // Generate new obstacles at top
        for (int x = (width - roadWidth) / 2 + 1; x < (width + roadWidth) / 2; x++) {
            gameBoard[0][x] = ' ';
        }
        
        // Add new obstacles but harder
        if (rand() % (10 - difficulty) == 0) {
            int obstaclePos = (width - roadWidth) / 2 + 1 + rand() % (roadWidth - 2);
            gameBoard[0][obstaclePos] = 'X';
        }
    }

    void increaseScore() {
        score++;
        
        // Increase difficulty each 10 points
        if (score % 10 == 0 && speed > 30) {
            speed -= 5;
            if (difficulty < 7) {
                difficulty++;
            }
        }
    }

    bool isGameOver() const {
        return gameOver;
    }

    int getScore() const {
        return score;
    }

    int getSpeed() const {
        return speed;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0)));
    
    std::cout << "Welcome to the C++ Racing Game!" << std::endl;
    std::cout << "Avoid obstacles (X) by moving your car (A) left and right." << std::endl;
    std::cout << "Press any key to start..." << std::endl;
    getch();
    
    Game game;
    char input = '\0';
    
    while (!game.isGameOver() && input != 'q') {
        game.drawBoard();
        game.updateObstacles();
        game.increaseScore();
        
        // Check for user input with timeout
        if (kbhit()) {
            input = getch();
            game.movePlayer(tolower(input));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(game.getSpeed()));
    }
    
    clearScreen();
    std::cout << "Game Over!" << std::endl;
    std::cout << "Final Score: " << game.getScore() << std::endl;
    std::cout << "Final Speed: " << (100 - game.getSpeed() + 10) << "mph" << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    getch();
    
    return 0;
}