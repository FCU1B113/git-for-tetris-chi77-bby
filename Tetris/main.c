#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 20

#define FALL_DELAY 500
#define RENDER_DELAY 100

// ��L��Ӫ�
#define LEFT_KEY 0x25
#define RIGHT_KEY 0x27 
#define ROTATE_KEY 0x26 
#define DOWN_KEY 0x28 
#define FALL_KEY 0x20 

// �P�_����O�_���Q���U���禡
#define LEFT_FUNC() GetAsyncKeyState(LEFT_KEY) & 0x8000
#define RIGHT_FUNC() GetAsyncKeyState(RIGHT_KEY) & 0x8000
#define ROTATE_FUNC() GetAsyncKeyState(ROTATE_KEY) & 0x8000
#define DOWN_FUNC() GetAsyncKeyState(DOWN_KEY) & 0x8000
#define FALL_FUNC() GetAsyncKeyState(FALL_KEY) & 0x8000



typedef enum
{
    RED = 41,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
    BLACK = 0,
} Color;

typedef enum
{
    EMPTY = -1,
    I,
    J,
    L,
    O,
    S,
    T,
    Z
} ShapeId;

typedef struct
{
    ShapeId shape;
    Color color;
    int size;
    char rotates[4][4][4];
} Shape;

typedef struct
{
    int x;
    int y;
    int score;
    int rotate;
    int fallTime;
    ShapeId queue[4];
} State;

typedef struct
{
    Color color;
    ShapeId shape;
    bool current;
} Block;

Shape shapes[7] = {
    {.shape = I,
     .color = CYAN,
     .size = 4,
     .rotates =
         {
             {{0, 0, 0, 0},
              {1, 1, 1, 1},
              {0, 0, 0, 0},
              {0, 0, 0, 0}},
             {{0, 0, 1, 0},
              {0, 0, 1, 0},
              {0, 0, 1, 0},
              {0, 0, 1, 0}},
             {{0, 0, 0, 0},
              {0, 0, 0, 0},
              {1, 1, 1, 1},
              {0, 0, 0, 0}},
             {{0, 1, 0, 0},
              {0, 1, 0, 0},
              {0, 1, 0, 0},
              {0, 1, 0, 0}}}},
    {.shape = J,
     .color = BLUE,
     .size = 3,
     .rotates =
         {
             {{1, 0, 0},
              {1, 1, 1},
              {0, 0, 0}},
             {{0, 1, 1},
              {0, 1, 0},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 1},
              {0, 0, 1}},
             {{0, 1, 0},
              {0, 1, 0},
              {1, 1, 0}}}},
    {.shape = L,
     .color = YELLOW,
     .size = 3,
     .rotates =
         {
             {{0, 0, 1},
              {1, 1, 1},
              {0, 0, 0}},
             {{0, 1, 0},
              {0, 1, 0},
              {0, 1, 1}},
             {{0, 0, 0},
              {1, 1, 1},
              {1, 0, 0}},
             {{1, 1, 0},
              {0, 1, 0},
              {0, 1, 0}}}},
    {.shape = O,
     .color = WHITE,
     .size = 2,
     .rotates =
         {
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}},
             {{1, 1},
              {1, 1}}}},
    {.shape = S,
     .color = GREEN,
     .size = 3,
     .rotates =
         {
             {{0, 1, 1},
              {1, 1, 0},
              {0, 0, 0}},
             {{0, 1, 0},
              {0, 1, 1},
              {0, 0, 1}},
             {{0, 0, 0},
              {0, 1, 1},
              {1, 1, 0}},
             {{1, 0, 0},
              {1, 1, 0},
              {0, 1, 0}}}},
    {.shape = T,
     .color = PURPLE,
     .size = 3,
     .rotates =
         {
             {{0, 1, 0},
              {1, 1, 1},
              {0, 0, 0}},

             {{0, 1, 0},
              {0, 1, 1},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 1},
              {0, 1, 0}},
             {{0, 1, 0},
              {1, 1, 0},
              {0, 1, 0}}}},
    {.shape = Z,
     .color = RED,
     .size = 3,
     .rotates =
         {
             {{1, 1, 0},
              {0, 1, 1},
              {0, 0, 0}},
             {{0, 0, 1},
              {0, 1, 1},
              {0, 1, 0}},
             {{0, 0, 0},
              {1, 1, 0},
              {0, 1, 1}},
             {{0, 1, 0},
              {1, 1, 0},
              {1, 0, 0}}}},
};

void setBlock(Block* block, Color color, ShapeId shape, bool current)
{
    block->color = color;
    block->shape = shape;
    block->current = current;
}

void resetBlock(Block* block)
{
    block->color = BLACK;
    block->shape = EMPTY;
    block->current = false;
}

void printCanvas(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
    printf("\033[0;0H\n");
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        printf("|");
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            printf("\033[%dm\u3000", canvas[i][j].color);
        }
        printf("\033[0m|\n");
    }
    // �L�X����
    printf("\033[%d;%dHScore: %d", 2, CANVAS_WIDTH * 2 + 5, state->score);
    // ��XNext:
    printf("\033[%d;%dHNext:", 3, CANVAS_WIDTH * 2 + 5);
    // ��X���ƻ���
    for (int i = 1; i <= 3; i++)
    {
        Shape shapeData = shapes[state->queue[i]];
        for (int j = 0; j < 4; j++)
        {
            printf("\033[%d;%dH", i * 4 + j, CANVAS_WIDTH * 2 + 15);
            for (int k = 0; k < 4; k++)
            {
                if (j < shapeData.size && k < shapeData.size && shapeData.rotates[0][j][k])
                {
                    printf("\x1b[%dm  ", shapeData.color);
                }
                else
                {
                    printf("\x1b[0m  ");
                }
            }
        }
    }
    return;
}

bool move(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], int originalX, int originalY, int originalRotate, int newX, int newY, int newRotate, ShapeId shapeId)
{
    Shape shapeData = shapes[shapeId];
    int size = shapeData.size;

    // �P�_������S�����ŦX����
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                // �P�_���S���X�h���
                if (newX + j < 0 || newX + j >= CANVAS_WIDTH || newY + i < 0 || newY + i >= CANVAS_HEIGHT)
                {
                    return false;
                }
                // �P�_���S���I��O�����
                if (!canvas[newY + i][newX + j].current && canvas[newY + i][newX + j].shape != EMPTY)
                {
                    return false;
                }
            }
        }
    }

    // ��������ª���m
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[originalRotate][i][j])
            {
                resetBlock(&canvas[originalY + i][originalX + j]);
            }
        }
    }

    // ���ʤ���ܷs����m
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (shapeData.rotates[newRotate][i][j])
            {
                setBlock(&canvas[newY + i][newX + j], shapeData.color, shapeId, true);
            }
        }
    }

    return true;
}

int clearLine(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
    // ���M���Ҧ� current �аO
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].current)
            {
                canvas[i][j].current = false;
            }
        }
    }

    int linesCleared = 0;
    int i = CANVAS_HEIGHT - 1;
    while (i >= 0)
    {
        bool isFull = true;
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (canvas[i][j].shape == EMPTY)
            {
                isFull = false;
                break;
            }
        }

        if (isFull)
        {
            linesCleared++;

            // ����v�橹�ⰼ�����ʵe
            int left = CANVAS_WIDTH / 2 - 1;
            int right = CANVAS_WIDTH / 2;

            while (left >= 0 || right < CANVAS_WIDTH)
            {
                if (left >= 0)
                {
                    resetBlock(&canvas[i][left]);
                    left--;
                }
                if (right < CANVAS_WIDTH)
                {
                    resetBlock(&canvas[i][right]);
                    right++;
                }

                printCanvas(canvas, state);
                Sleep(100);
            }

            // �N�W�������U���@��
            for (int j = i; j > 0; j--)
            {
                for (int k = 0; k < CANVAS_WIDTH; k++)
                {
                    canvas[j][k] = canvas[j - 1][k];
                }
            }

            // �M�ų̳���
            for (int k = 0; k < CANVAS_WIDTH; k++)
            {
                resetBlock(&canvas[0][k]);
            }

            // �����@��ᤣ���W���A�]���W�������U�ӫ��٭n�ˬd�o�@��
            // �ҥH i ����
        }
        else
        {
            i--;  // �o�椣���A���W�ˬd�U�@��
        }
    }

    return linesCleared;
}



void showStartScreen() {
    system("cls");
    printf("\033[0;0H");
    printf("\n\n\n\n");
    printf("      \033[36mTETRIS �C��\033[0m\n");
    printf("  -------------------------\n");
    printf("     �� ���G���ʤ��\n");
    printf("     ���G������\n");
    printf("     ���G�ֳt�U��\n");
    printf("     �ť���G�������U\n");
    printf("  -------------------------\n");
    printf("     \033[32m�����N��}�l...\033[0m\n");
    while (!_kbhit()) {
        Sleep(100);
    }
    _getch(); // ���ݥ��N��
}

void showGameOverScreen(int score) {
    system("cls");
    printf("\033[0;0H");
    printf("\n\n\n\n");
    printf("      \033[41m GAME OVER \033[0m\n");
    printf("  -------------------------\n");
    printf("     �A�����ơG%d\n", score);
    printf("  -------------------------\n");
    printf("     \033[33m�����N�����}...\033[0m\n");
    while (!_kbhit()) {
        Sleep(100);
    }
    _getch();
    exit(0);
}

void fadeOutFullScreen() {
    // ���o��ӵe������ (�]�t�C�����M�������)
    int totalRows = CANVAS_HEIGHT + 10; // �A�i�H�ھڹ�ڵe���վ㰪��

    // �C��q�W���U�л\�ť�
    for (int row = 0; row < totalRows; row++) {
        // ���ʴ�Ш�Ӧ�_�l��m
        printf("\033[%d;0H", row + 1); // ANSI escape codes�A��Ʊq1�}�l

        // ��X�ťն񺡾��]���]80�r���e�ס^
        for (int i = 0; i < 80; i++) {
            printf(" ");
        }
        fflush(stdout);

        Sleep(50); // �Ȱ� 50 �@��A�C�C�����ĪG
    }
}


void logic(Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH], State* state)
{
    if (ROTATE_FUNC())
    {
        int newRotate = (state->rotate + 1) % 4;
        if (move(canvas, state->x, state->y, state->rotate, state->x, state->y, newRotate, state->queue[0]))
        {
            state->rotate = newRotate;
        }
    }
    else if (LEFT_FUNC())
    {
        if (move(canvas, state->x, state->y, state->rotate, state->x - 1, state->y, state->rotate, state->queue[0]))
        {
            state->x -= 1;
        }
    }
    else if (RIGHT_FUNC())
    {
        if (move(canvas, state->x, state->y, state->rotate, state->x + 1, state->y, state->rotate, state->queue[0]))
        {
            state->x += 1;
        }
    }
    else if (DOWN_FUNC())
    {
        state->fallTime = FALL_DELAY;
    }
    else if (FALL_FUNC())
    {
        state->fallTime += FALL_DELAY * CANVAS_HEIGHT;
    }

    state->fallTime += RENDER_DELAY;

    while (state->fallTime >= FALL_DELAY)
    {
        state->fallTime -= FALL_DELAY;
        if (move(canvas, state->x, state->y, state->rotate, state->x, state->y + 1, state->rotate, state->queue[0]))
        {
            state->y++;
        }
        else
        {
            state->score += clearLine(canvas, state);


            state->x = CANVAS_WIDTH / 2;
            state->y = 0;
            state->rotate = 0;
            state->fallTime = 0;
            state->queue[0] = state->queue[1];
            state->queue[1] = state->queue[2];
            state->queue[2] = state->queue[3];
            state->queue[3] = rand() % 7;

            //������X
            if (!move(canvas, state->x, state->y, state->rotate, state->x, state->y, state->rotate, state->queue[0]))
            {
                printf("\033[%d;%dH\x1b[41m GAME OVER \x1b[0m\033[%d;%dH", CANVAS_HEIGHT - 3, CANVAS_WIDTH * 2 + 5, CANVAS_HEIGHT + 5, 0);
                fadeOutFullScreen();
                showGameOverScreen(state->score);
            }
        }
    }
    return;
}

int main()
{
    showStartScreen();
    srand(time(NULL));
    State state = {
        .x = CANVAS_WIDTH / 2,
        .y = 0,
        .score = 0,
        .rotate = 0,
        .fallTime = 0 };

    for (int i = 0; i < 4; i++)
    {
        state.queue[i] = rand() % 7;
    }

    Block canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            resetBlock(&canvas[i][j]);
        }
    }

    Shape shapeData = shapes[state.queue[0]];

    for (int i = 0; i < shapeData.size; i++)
    {
        for (int j = 0; j < shapeData.size; j++)
        {
            if (shapeData.rotates[0][i][j])
            {
                setBlock(&canvas[state.y + i][state.x + j], shapeData.color, state.queue[0], true);
            }
        }
    }

    while (1)
    {
        printCanvas(canvas, &state);
        logic(canvas, &state);
        Sleep(100);
    }

    return 0;
}