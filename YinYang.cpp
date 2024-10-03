#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QPushButton>
#include <stack>
#include <vector>

// Define the colors
#define WHITE 0
#define BLACK 1

// Function to get the color of a circle at a given position
unsigned int getColor(unsigned int state, int pos) {
    return (state >> (pos + 16)) & 1;
}

// Function to set the color of a circle at a given position
unsigned int setColor(unsigned int state, int pos, int color) {
    if (color == BLACK) {
        return state | (1 << (pos + 16));
    } else {
        return state & ~(1 << (pos + 16));
    }
}

// Function to check the existence of a circle at a given position
bool exists(unsigned int state, int pos) {
    return (state >> pos) & 1;
}

// Function to set the existence of a circle at a given position
unsigned int setExistence(unsigned int state, int pos, bool existence) {
    if (existence) {
        return state | (1 << pos);
    } else {
        return state & ~(1 << pos);
    }
}

// Function to perform a depth-first search on the board
void dfsConnectivity(unsigned int state, int pos, int color, std::vector<bool>& visited) {
    int row = pos / 4;
    int col = pos % 4;

    // Directions for the neighboring cells (up, down, left, right)
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    // Mark the current cell as visited
    visited[pos] = true;

    // Visit all unvisited neighboring cells of the same color
    for (int i = 0; i < 4; i++) {
        int newRow = row + dr[i];
        int newCol = col + dc[i];
        int newPos = newRow * 4 + newCol;
        if (newRow >= 0 && newRow < 4 && newCol >= 0 && newCol < 4 &&
            !visited[newPos] && (getColor(state, newPos) == color || !exists(state, newPos))) {
            dfsConnectivity(state, newPos, color, visited);
        }
    }
}

// Function to check if a state is valid according to the rules of the Yin Yang puzzle
bool isValid(unsigned int state, int pos, bool isBFS) {
    // Rule 2: All white circles and all black circles should be orthogonally connected
    for (int color = WHITE; color <= BLACK; color++) {
        std::vector<bool> visited(16, false);
        bool foundColor = false;
        for (int i = 0; i < 16; i++) {
            if (getColor(state, i) == color) {
                dfsConnectivity(state, i, color, visited);
                foundColor = true;
                break;
            }
        }
        if (foundColor) {
            for (int i = 0; i < 16; i++) {
                if (exists(state, i)) {
                    if ((getColor(state, i) == color) && !visited[i]) {
                        return false;
                    }
                }
            }
        }
    }

    // Rule 3: There may not be any 2x2 cell region consisting of the same circle color
    int row = pos / 4;
    int col = pos % 4;
    for (int i = -1; i <= 0; i++) {
        for (int j = -1; j <= 0; j++) {
            int newRow = row + i;
            int newCol = col + j;
            if (newRow >= 0 && newRow < 3 && newCol >= 0 && newCol < 3) {
                if (exists(state, newRow * 4 + newCol) && exists(state, newRow * 4 + newCol + 1) &&
                    exists(state, (newRow + 1) * 4 + newCol) && exists(state, (newRow + 1) * 4 + newCol + 1) &&
                    getColor(state, newRow * 4 + newCol) == getColor(state, newRow * 4 + newCol + 1) &&
                    getColor(state, newRow * 4 + newCol) == getColor(state, (newRow + 1) * 4 + newCol) &&
                    getColor(state, newRow * 4 + newCol) == getColor(state, (newRow + 1) * 4 + newCol + 1)) {
                    return false;
                }
            }
        }
    }

    return true;
}


// Function to convert a state to a string representation
std::string stateToString(unsigned int state) {
    std::string result;
    for (int i = 0; i < 16; i++) {
        if (exists(state, i)) {
            if (getColor(state, i) == BLACK) {
                result += 'B';
            } else {
                result += 'W';
            }
        } else {
            result += '-';  // No circle at this position
        }
    }
    return result;
}

// Function to create a new QGraphicsScene for a given state
QGraphicsScene* createScene(unsigned int state, int cellSize) {
    QGraphicsScene* scene = new QGraphicsScene;

    // Add rectangles representing the cells to the scene
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int pos = row * 4 + col;
            QGraphicsRectItem* rectItem = new QGraphicsRectItem(col * cellSize, row * cellSize, cellSize, cellSize);

            // Set the color based on the state
            if (exists(state, pos) && getColor(state, pos) == BLACK) {
                rectItem->setBrush(QBrush(Qt::black));
            } else if (exists(state, pos) && getColor(state, pos) == WHITE) {
                rectItem->setBrush(QBrush(Qt::white));
            } else {
                rectItem->setBrush(QBrush(Qt::transparent));  // No circle at this position
            }

            scene->addItem(rectItem);
        }
    }

    return scene;
}

// Function to generate all valid states using DFS
std::vector<std::string> generateValidStates() {
    std::vector<std::string> validStates;
    unsigned int initialState = 0;  // Initial state with no circles
    std::stack<unsigned int> stateStack;
    stateStack.push(initialState);

    while (!stateStack.empty()) {
        unsigned int state = stateStack.top();
        stateStack.pop();

        int pos = -1;
        for (int i = 0; i < 16; i++) {
            if (!exists(state, i)) {
                pos = i;
                break;
            }
        }

        if (pos == -1) {
            if (isValid(state, 16, false)) {
                validStates.push_back(stateToString(state));
            }
        } else {
            for (int color = WHITE; color <= BLACK; color++) {
                unsigned int newState = setExistence(setColor(state, pos, color), pos, true);
                if (isValid(newState, pos, false)) {
                    stateStack.push(newState);
                }
            }
        }
    }
    return validStates;
}

class MainWindow : public QMainWindow {
public:
    MainWindow(const std::vector<std::string>& states, QWidget *parent = nullptr) : QMainWindow(parent) {
        QScrollArea* scrollArea = new QScrollArea(this);
        QWidget* container = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(container);

        for (size_t i = 0; i < states.size(); ++i) {
            QPushButton* button = new QPushButton(QString("State %1").arg(i + 1), this);
            connect(button, &QPushButton::clicked, this, [this, i, states]() { openStateWindow(states[i]); });
            layout->addWidget(button);
        }

        container->setLayout(layout);
        scrollArea->setWidget(container);
        scrollArea->setWidgetResizable(true);
        setCentralWidget(scrollArea);
    }

private:
    void openStateWindow(const std::string& stateStr) {
        unsigned int state = 0;
        for (int i = 0; i < 16; i++) {
            if (stateStr[i] == 'B') {
                state = setExistence(setColor(state, i, BLACK), i, true);
            } else if (stateStr[i] == 'W') {
                state = setExistence(setColor(state, i, WHITE), i, true);
            }
        }

        QGraphicsScene* scene = createScene(state, 100);
        QGraphicsView* view = new QGraphicsView(scene);
        view->setFixedSize(400, 400);
        view->setAttribute(Qt::WA_DeleteOnClose); // Ensure memory is freed when window is closed
        view->show();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    std::vector<std::string> validStates = generateValidStates();

    MainWindow mainWindow(validStates);
    mainWindow.resize(600, 400);
    mainWindow.show();

    return app.exec();
}