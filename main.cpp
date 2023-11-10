#include <unistd.h>
#include <iostream>
#include <ncurses.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>

using namespace std;

class GameObject {
public:
    int x, y;
};

class Score: public GameObject {
public:
    int num;

    void gotoxy(int x, int y) {
        printf("%c[%d;%df", 0x1B, y, x);
    }
};

struct Player: public GameObject {
    int speed;
    char symbol; // символ, отображающий игрока на игровом поле
};

class Traffic: public GameObject {
public:
    char symbol;

    void changePlaceTraffic(int time, Traffic &traffic, vector<Traffic> &trafficVector) {
        for (int i = 0; i < 3; i++) {
            trafficVector[i].y++;
        }
    }
    void setPlaceTraffic(int width, Traffic &traffic, Score &score, vector<Traffic> &trafficVector) {
    for (int i = 0; i < 3; i++) {
        trafficVector[i].y = 0;
        trafficVector[i].x = 1 + (rand() % (width - 2));
        score.num++;
    }
}
};

class MysteryBox: public GameObject {
public:
    int fine;
    char symbol;
    string feautures[2];
    string feauture;

    void setPlaceBox(int width, int height, MysteryBox &box, Traffic &traffic, Score &score) {
        box.feauture = box.feautures[(rand() % 2)];
        box.x = 1 + (rand() % (width - 2));
        box.y = 1 + (rand() % (height - 2));
        if (box.x == traffic.x) {
            box.x = 1 + (rand() % (width - 2));
        }
        if (box.feauture == box.feautures[1]) {
            score.num++;
        }
    }
};

class Shuttle: public GameObject {
public:
    char symbol;
    bool moveRight;

    void changePlaceShuttle(int width, int height, Shuttle &shuttle) {
        if (shuttle.moveRight == true) {
            if (shuttle.x == width - 2) {
                shuttle.moveRight = false;
                shuttle.x--;
            } else {
                shuttle.x++;
            }
        } else {
            if (shuttle.x == 1) {
                shuttle.moveRight = true;
                shuttle.x++;
            } else {
                shuttle.x--;
            }
        }

    }
};

class Bullet: public GameObject {
public:
    char symbol;

    void changePlaceBullet(Bullet &bullet) {
        bullet.y++;
    }
    void setPlaceBullet(int width, Bullet &bullet, Shuttle &shuttle, Score &score) {
        bullet.x = shuttle.x;
        bullet.y = shuttle.y + 1;
        score.num++;
    }
};

class PlayerBullet: public Bullet {
public:
    int flag;

    void changePlacePlayerBullet(PlayerBullet &pbullet) {
        pbullet.y--;
    }

    void setPlacePlayerBullet(int width, PlayerBullet &pbullet, Player &player, Score &score) {
        pbullet.x = player.x;
        pbullet.y = player.y - 1;
    }
};

class BestScore {
public:
    string readFile() {
        ifstream fin("score.txt");
        if (!fin) {
            cout << "Error!" << endl;
            cin.get();
        }
        string scorefromTxtfile;
        getline(fin, scorefromTxtfile);
        return scorefromTxtfile;
    }
    void resultScore(Score &score) {
        if (score.num > stoi(readFile())) {
            std::ofstream out;          // поток для записи
            out.open("score.txt");      // открываем файл для записи
            if (out.is_open())
            {
                out << score.num << std::endl;
            }
            out.close(); 
            cout << "Your best score: " << score.num;
        } else {
            cout << "Your best score: " << readFile();
        }
    }
};

void playerController(Player &player, int width, int height, bool gameOver, MysteryBox &box, PlayerBullet &pbullet, Shuttle &shuttle, Score &score) {
    int input = getch(); // ожидание ввода пользователя

    if (input == 'a' && player.x > 1) {
        player.x--; // движение влево
    }
    if (input == 'd' && player.x < width - 2) {
        player.x++; // движение вправо
    }
    if (input == 'w' && player.y > 0) { // движение вверх
        if (player.speed == 60) {
            player.y = player.y - 1;    
        } else if (player.speed == 120) {
            if (box.fine == 0) {
                player.speed = 60;
                player.y --;
            } else {
                player.y = player.y - 2;
                box.fine --;
            }
            
        } else if (player.speed == 0) {
            if (box.fine == 0) {
                player.speed = 60;
                player.y --;
            } else {
                player.y = player.y - 0;
                box.fine --;
            }
        }

    }
    if (input == 's' && player.y < height - 1) {
        player.y++; // движение вниз
    }
    if (input == 'e') { //пуля игрока
        pbullet.flag = 1;
        pbullet.setPlacePlayerBullet(width, pbullet, player, score);
    }
    if (input == 'x') {
        gameOver = true; // закончить игру при нажатии клавиши 'x'
    }
}

class GameBoard {
private:
    static const int height = 18; // ширина игрового поля
    static const int width = 27; // высота игрового поля
public:
    int getHeight() const {
        return height;
    }
    int getWidth() const {
        return width;
    }
    char map[height][width];
    void initializingPlayingField() {
        // Инициализация игрового поля
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (j == width - 1 || j == 0 || j == width) {
                    map[i][j] = '#'; // отрисовка дороги
                } else {
                    map[i][j] = ' '; // отрисовка пустого пространства
                }
            }
        }
        
        // Настройка ncurses для отрисовки игрового поля
        initscr();
        curs_set(0); // скрыть курсор
        noecho(); // не отображать вводимые символы на экране
    }
    
    void drawingPlayingField(Player& player, vector<Traffic>& trafficVector, MysteryBox& box, PlayerBullet& pbullet, Bullet& bullet, Shuttle& shuttle) {
        // Обновление игрового поля
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                for (int k = 0; k < 3; k++) {
                    if (i == player.y && j == player.x) {
                        map[i][j] = player.symbol; // отрисовка машинки
                    } else if (i == trafficVector[k].y && j == trafficVector[k].x) {
                        for (int r = 0; r < 3; r++) {
                            map[i][j] = trafficVector[r].symbol; // отрисовка трафика
                        }
                    } else if (i == shuttle.y && j == shuttle.x) {
                        map[i][j] = shuttle.symbol; // отрисовка корабля
                    } else if (i == bullet.y && j == bullet.x) {
                        map[i][j] = bullet.symbol; // отрисовка пули
                    } else if (i == pbullet.y && j == pbullet.x) {
                        map[i][j] = pbullet.symbol; // отрисовка пули игрока
                    } else if (i == box.y && j == box.x) {
                        map[i][j] = box.symbol; // отрисовка mysterybox
                    } else {
                        if (j == width - 1 || j == 0 || j == width) {
                            map[i][j] = '#'; // отрисовка дороги
                        } else {
                            for (int r = 0; r < 3; r++) {        
                                if (i != trafficVector[r].y && j != trafficVector[r].x) {
                                    map[i][j] = ' '; // отрисовка пустого пространства
                                }
                            }
                        }
                    }
                    refresh();
                }
            }
        }
    }
};

int main() {
    //создаем объекты классов
    GameBoard board;
    Player player; 
    Traffic traffic;
    MysteryBox box;
    Shuttle shuttle;
    Bullet bullet;
    PlayerBullet pbullet;
    Score score;

    //массив с объектами траффика (3 объекта)
    vector<Traffic> trafficVector;
    for (int i = 0; i < 3; i++) {
        Traffic traffic;
        traffic.x = 1 + (rand() % (board.getWidth() - 2));
        traffic.y = 0;
        traffic.symbol = '*';
        trafficVector.push_back(traffic);
    }

    board.initializingPlayingField();

    score.num = 0;
    score.x = board.getWidth() + 2;
    score.y = 1;

    player.x = board.getWidth() / 2; // начальное положение машинки по горизонтали (по середине дороги)
    player.y = board.getHeight() - 1; // начальное положение машинки по вертикали (перед дорогой)
    player.symbol = '^'; // символ машинки
    player.speed = 60;

    traffic.x = 1 + (rand() % (board.getWidth() - 2));
    traffic.y = 0;
    traffic.symbol = '*';

    box.x = 1 + (rand() % (board.getWidth() - 2));
    box.y = 1 + (rand() % (board.getHeight() - 2));
    box.symbol = 'o';
    box.feautures[0] = "boost";
    box.feautures[1] = "lag";
    box.fine = 0;
    if (box.x == traffic.x) {
        box.x -= 1;
    }
    box.feauture = box.feautures[(rand() % 2)];
    for (int i = 0; i < 3; i++) {
        if (box.x == trafficVector[i].x) {
            box.x -= 1;
        }
    }

    shuttle.x = 1;
    shuttle.y = 0;
    shuttle.symbol = '~';
    shuttle.moveRight = true;

    bullet.x = -2;
    bullet.symbol = '|';

    pbullet.y = -2;
    pbullet.symbol = ':';
    pbullet.flag = 0;

    bool gameOver = false; // флаг окончания игры

    // Основной игровой цикл
    while (!gameOver) {
        // Отрисовка игрового поля
        for (int i = 0; i < board.getHeight(); i++) {
            for (int j = 0; j < board.getWidth(); j++) {
                mvprintw(i, j, "%c", board.map[i][j]);
            }
        }

        //управление
        playerController(player, board.getWidth(), board.getHeight(), gameOver, box, pbullet, shuttle, score);
        //отрисовка элементов на карте
        board.drawingPlayingField(player, trafficVector, box, pbullet, bullet, shuttle);

        //столкновение с mysteryBox
        if (player.x == box.x && (player.y == box.y || player.y == box.y - 1)) {
            if (box.feauture == "boost") {
                player.speed = 120;
                box.fine = 3;

            } else if (box.feauture == "lag") {
                player.speed = 0;
                box.fine = 5;
            }
        }
        
        //столкновение с препятствием
        for (int i = 0; i < 3; i++) {
            if (player.x == trafficVector[i].x && (player.y == trafficVector[i].y || player.y == trafficVector[i].y - 2 || player.y == traffic.y - 1)) {
                gameOver = true;
            }
        }
        //столкновение пули с шатлом
        if (pbullet.x == shuttle.x && pbullet.y == shuttle.y) {
            shuttle.x = 0;
            score.num++;
        }
        //столкновение с пулей
        if (player.x == bullet.x && (player.y == bullet.y || player.y == bullet.y - 2 || player.y == bullet.y - 1)) {
            gameOver = true;
        }
        //спавн пули
        int point = (int) board.getHeight()/3;
        if (trafficVector[0].y == point) {
            bullet.setPlaceBullet(board.getWidth(), bullet, shuttle, score);
        }
        //респавн траффика
        for (int i = 0; i < 3; i++) {
            if (trafficVector[i].y == board.getHeight()) {
                traffic.setPlaceTraffic(board.getWidth(), traffic, score, trafficVector);
            }
        }
        //респавн мистери бокса
        if (player.x == box.x && (player.y == box.y || player.y == box.y - 1)) {
            box.setPlaceBox(board.getWidth(), board.getHeight(), box, traffic, score);
        }

        //шаг элементов вниз
        traffic.changePlaceTraffic(1, traffic, trafficVector);
        shuttle.changePlaceShuttle(board.getWidth(), board.getHeight(), shuttle);
        bullet.changePlaceBullet(bullet);

        if (pbullet.flag == 1) {
            pbullet.changePlacePlayerBullet(pbullet);
        }
        if (pbullet.y == -2) {
            pbullet.flag = 0;
        }

        score.gotoxy(board.getWidth() + 2, 2);
        cout << "Score: " << score.num;
        
    }
    endwin(); // завершение работы ncurses
    cout << "Game Over!" << endl;

    BestScore bestScore;
    bestScore.resultScore(score);

    return 0;
}