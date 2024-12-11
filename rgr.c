#include <ncurses.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>


#define KEY_ENTER 10
#define KEY_ESCAPE 27
#define KEY_BACKSPACE 127

#define RED_TEXT COLOR_PAIR(1)
#define GREEN_TEXT COLOR_PAIR(2)
#define YELLOW_TEXT COLOR_PAIR(2)

#define MAX_NAME 50
#define MAX_FILES 100
#define MAX_FILEPATH 200
#define MAX_BOOKS 300
#define MAX_PAGES 1500
#define MAX_PRICE 100000

#define BASE_X 2
#define BASE_Y 2

struct Book {
    char name[MAX_NAME];
    int pages;
    int price;
};

struct Book books[MAX_BOOKS];
struct Book *bookPointers[MAX_BOOKS];

void waitEnter() {
    while (1) if (getch() == KEY_ENTER) break;
}

void displaySuccessMessage(char *message) {
    clear();
    attron(GREEN_TEXT);
    mvprintw(BASE_Y, BASE_X, "%s", message);
    attroff(GREEN_TEXT);
    waitEnter();
}

int inputNumber(const char *placeholder, const int minValue, const int maxValue) {
    int number = 0;
    char *validation = "";

    while (1) {
        clear();
        mvprintw(BASE_Y, BASE_X, "%s", placeholder);
        mvprintw(BASE_Y, BASE_X + strlen(placeholder), "%d", number);
        attron(RED_TEXT);
        mvprintw(BASE_Y + 1, BASE_X, "%s", validation);
        attroff(RED_TEXT);
        refresh();
        const int key = getch();

        if (isdigit(key)) {
            validation = "";
            number = number * 10 + (key - '0');
        } else if (key == KEY_ENTER) {
            if (number < minValue) {
                validation = "Number is too small";
            } else if (number > maxValue) {
                validation = "Number is too big";
            } else {
                break;
            }
        } else if (key == KEY_BACKSPACE) {
            validation = "";
            number /= 10;
        } else if (key == KEY_ESCAPE) {
            return -1;
        }
    }

    return number;
}

void inputString(char *buffer, const char *placeholder, const int maxLength) {
    int cursor = 0;
    char *validation = "";

    while (1) {
        clear();
        mvprintw(BASE_Y, BASE_X, "%s", placeholder);
        mvprintw(BASE_Y, BASE_X + strlen(placeholder), "%s", buffer);

        attron(RED_TEXT);
        mvprintw(BASE_Y + 1, BASE_X, "%s", validation);
        attroff(RED_TEXT);
        refresh();

        int key = getch();

        if (isprint(key) && cursor < maxLength) {
            validation = "";
            buffer[cursor++] = key;
            buffer[cursor] = '\0'; // Null-terminate the string
        } else if (key == KEY_BACKSPACE) {
            validation = "";
            if (cursor > 0) {
                buffer[--cursor] = '\0'; // Remove last character
            }
        } else if (key == KEY_ENTER) {
            if (cursor == 0) {
                validation = "Input cannot be empty.";
            } else {
                break;
            }
        } else if (cursor >= maxLength) {
            validation = "Input is too long.";
        }
    }
}

int listFiles(const char *path, char *files[], int maxFiles) {
    struct stat pathStat;

    if (stat(path, &pathStat) != 0) {
        perror("Error accessing path");
    }

    if (S_ISDIR(pathStat.st_mode)) {
        struct dirent *entry;

        DIR *directory = opendir(path);
        if (directory == NULL) {
            return -1;
        }
        int i = 0;
        while ((entry = readdir(directory)) != NULL) {
            if (i == maxFiles) break;
            if (strcmp(entry->d_name, ".") != 0) {
                files[i] = strdup(entry->d_name);
                i++;
            }
        }

        closedir(directory);
        return i;
    }
    return 1;
}


void displayMenu(char *placeholder, const char *titles[], const int menuSize, int highlight) {
    int y = BASE_Y;

    clear();
    mvprintw(0, BASE_X, "%s", placeholder);
    for (int i = 0; i < menuSize; i++) {
        if (i == highlight) {
            attron(A_UNDERLINE);
            mvprintw(y++, BASE_X, "%s", titles[i]);
            attroff(A_UNDERLINE);
        } else {
            mvprintw(y++, BASE_X, "%s", titles[i]);
        }
    }
    refresh();
}

int menuSelection(char *placeholder, const char *titles[], const int menuSize) {
    unsigned int selection = 0;
    int choice = -1;

    while (1) {
        displayMenu(placeholder, titles, menuSize, selection);
        const int key = getch();
        switch (key) {
            case KEY_UP:
                selection = (menuSize + selection - 1) % menuSize;
                break;
            case KEY_DOWN:
                selection = (menuSize + selection + 1) % menuSize;
                break;
            case KEY_ENTER:
                choice = selection;
                break;
            default:
                break;
        }

        if (choice != -1) {
            clear();
            return choice;
        }
    }
}

void selectFile(char *path) {
    const char file[MAX_FILEPATH] = ".";


    const char *files[MAX_FILES + 3];

    while (1) {
        char placeholder[150];
        char absolute_path[1000];

        const int filesCount = listFiles(file, files, MAX_FILES);

        files[filesCount] = "Save";
        files[filesCount + 1] = "New file";
        files[filesCount + 2] = "Exit";

        if (realpath(file, absolute_path) == NULL) {
            perror("Error resolving absolute path");
        }

        strcpy(placeholder, "User arrow keys to navigate. Enter to select. Select save for apply selection.\n  ");
        strcat(placeholder, absolute_path);

        const int choice = menuSelection(placeholder, files, filesCount + 3);

        if (choice == filesCount) {
            realpath(file, absolute_path);
            strcpy(path, absolute_path);
            return;
        }
        if (choice == filesCount + 1) {
            char newFile[MAX_FILEPATH] = "";
            inputString(newFile, "Enter new file name: ", MAX_FILEPATH);
            snprintf(file, MAX_FILEPATH, "%s/%s", file, newFile);
            realpath(file, absolute_path);
            strcpy(path, absolute_path);
            return;
        }
        if (choice == filesCount + 2) {
            path[0] = '\0';
            return;
        }
        snprintf(file, MAX_FILEPATH, "%s/%s", file, files[choice]);
    }
}

void writeToCSVFile(const char *filePath) {
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        perror("Error creating file");
    }

    fprintf(file, "Index,Name,Pages,Price\n");

    for (int i = 0; i < 300; ++i) {
        if (bookPointers[i]->name[0] == '\0') continue;
        fprintf(file, "%d,%s,%d,%d\n", i, bookPointers[i]->name, bookPointers[i]->pages, bookPointers[i]->price);
    }

    fclose(file);
}

void readFromCSVFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char header[100];
    fgets(header, 100, file);

    for (int i = 0; i < 300; ++i) {
        int index;
        char name[MAX_NAME];
        int pages = 0;
        int price = 0;
        fscanf(file, "%d,%49[^,],%d,%d", &index, name, &pages, &price);
        strcpy(bookPointers[index]->name, name);
        bookPointers[index]->pages = pages;
        bookPointers[index]->price = price;
    }

    fclose(file);
}

void initNcurses() {
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
}


struct MenuOption {
    char *title;

    void (*func)();
};

void sortBooks() {
    for (int i = 0; i < MAX_BOOKS - 1; i++) {
        for (int j = i + 1; j < MAX_BOOKS; j++) {
            if (bookPointers[i]->name[0] == '\0' ||
                (bookPointers[j]->name[0] != '\0' && bookPointers[i]->price > bookPointers[j]->price)) {
                struct Book *temp = bookPointers[i];
                bookPointers[i] = bookPointers[j];
                bookPointers[j] = temp;
            }
        }
    }

    displaySuccessMessage("Books sorted");
}

void inputBook() {
    const int id = inputNumber("Enter book index: ", 0, MAX_BOOKS - 1);
    inputString(bookPointers[id]->name, "Enter book name: ", MAX_NAME);
    bookPointers[id]->pages = inputNumber("Enter book pages: ", 1, MAX_PAGES);
    bookPointers[id]->price = inputNumber("Enter book price: ", 0, MAX_PRICE);
    displaySuccessMessage("Book data entered");
}

void outputBook() {
    int y = BASE_Y;

    const int id = inputNumber("Enter book id: ", 0, MAX_BOOKS - 1);

    clear();

    mvprintw(y, BASE_X, "Name: %s\n", bookPointers[id]->name);
    mvprintw(++y, BASE_X, "Pages: %d\n", bookPointers[id]->pages);
    mvprintw(++y, BASE_X, "Price: %d\n", bookPointers[id]->price);
    waitEnter();
}

void drawTableDelimiters(const int y, const int x, const int width, const int steps, const chtype delimiter) {
    for (int step = 0; step <= steps; step++) {
        mvaddch(y, x + step * width, delimiter);
    }
}

void drawTableFrameHLine(const int y, const int x, const int width, const int steps, const chtype start,
                         const chtype end, const chtype breakout) {
    mvhline(y, x, 0, steps * width);
    drawTableDelimiters(y, x, width, steps, breakout);
    mvaddch(y, x, start);
    mvaddch(y, x + steps * width, end);
}

void outputAllBooks(char *filter) {
    unsigned int y = BASE_Y;
    const unsigned int cols = 4;
    const unsigned int width = 20;

    const char *headers[4] = {"Index", "Name", "Pages", "Price"};

    drawTableFrameHLine(y, BASE_X, width, cols, ACS_ULCORNER, ACS_URCORNER, ACS_TTEE);
    mvaddch(y + 1, BASE_X, ACS_VLINE);
    mvaddch(y + 1, BASE_X + cols * width, ACS_VLINE);

    for (int col = 0; col < cols; col++) {
        mvprintw(y + 1, BASE_X + 1 + col * width, "%-*s", width - 2, headers[col]);
    }

    drawTableDelimiters(y + 1, BASE_X, width, cols, ACS_VLINE);

    int rowY = y + 3;
    for (int i = 0; i < MAX_BOOKS; i++) {
        if (bookPointers[i]->name[0] == '\0') continue;
        int shift = 0;
        mvprintw(rowY, BASE_X + 1, "%-*d", width - 2, i);
        mvprintw(rowY, BASE_X + 1 + width * ++shift, "%-*s", width - 2, bookPointers[i]->name);
        mvprintw(rowY, BASE_X + 1 + width * ++shift, "%-*d", width - 2, bookPointers[i]->pages);
        mvprintw(rowY, BASE_X + 1 + width * ++shift, "%-*d", width - 2, bookPointers[i]->price);
        drawTableDelimiters(rowY, BASE_X, width, cols, ACS_VLINE);
        drawTableFrameHLine(rowY - 1, BASE_X, width, cols, ACS_LTEE, ACS_RTEE, ACS_PLUS);
        rowY += 2;
    }
    drawTableFrameHLine(rowY - 1, BASE_X, width, cols, ACS_LLCORNER, ACS_LRCORNER, ACS_BTEE);

    waitEnter();
}

void writeToFile() {
    char file[MAX_FILEPATH];
    selectFile(file);
    writeToCSVFile(file);
    displaySuccessMessage("File written");
}

void readFromFile() {
    char file[MAX_FILEPATH];
    selectFile(file);
    readFromCSVFile(file);
    displaySuccessMessage("File read");
}


void menuChoice(const struct MenuOption *menuOptions, const int menuSize) {
    char *titles[menuSize];

    for (int i = 0; i < menuSize; i++) {
        titles[i] = menuOptions[i].title;
    }

    // ReSharper disable once CppDFAEndlessLoop
    while (1) {
        const int choice = menuSelection(
            "Select file. User arrow keys to navigate. Enter to select.", titles,
            menuSize
        );

        clear();
        menuOptions[choice].func();
    }
}

int main() {
    const struct MenuOption menuOptions[7] = {
        {
            .title = "Input book data",
            .func = inputBook,
        },
        {
            .title = "Output book data",
            .func = outputBook,
        },
        {
            .title = "Sort books by price",
            .func = sortBooks,
        },
        {
            .title = "Print all books",
            .func = outputAllBooks,
        },
        {
            .title = "Write to file",
            .func = writeToFile
        },
        {
            .title = "Read from file",
            .func = readFromFile
        },
        {
            .title = "Exit",
            .func = exit
        }
    };

    for (int i = 0; i < MAX_BOOKS; i++) {
        bookPointers[i] = &books[i];
    }

    initNcurses();

    menuChoice(menuOptions, 7);

    return 0;
}
