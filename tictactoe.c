
#include <linux/module.h>

#include <linux/fs.h>

#include <asm/uaccess.h>

#include <linux/miscdevice.h>

#include <linux/random.h>

#include <linux/init.h>

#include <linux/errno.h>

#include <linux/string.h>

#include <linux/slab.h>

#include <linux/kernel.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jedan Davis");
MODULE_DESCRIPTION("A simple Linux kernel module that provides inspirational quotes");
MODULE_VERSION("0.1");

// message handling
#define MSG_INVALID_MOVE 0
#define MSG_GAME_OVER_TIE 1
#define MSG_PLAYER_WON 2
#define MSG_RESET 3
#define MSG_INVALID_COMMAND 4
#define MSG_AI_WON 5
#define MSG_BOARD 6

int message = -1;
int message2 = 0;

// global variables
static bool IsAiTurn = false;
static bool continue_game = true;
char board[3][3];

// message handling array
const char *messages[] = {
    "Invalid move! Please enter a valid move.\n",
    "Game over! It's a tie!\n",
    "Congratulations, Player X has won!\n",
    "OK\n",
    "Invalid command! Please enter a valid command.\n",
    "Congratulations, Player O has won!\n"};

// prototypes
static void init_board(void);
static int isFull(void);
static void WhoWon(char);
static int checkWin(char);
static int validMove(int, int);
static void resetBoard(void);
static void makeMove(int, char, char);
static void aiMove(void);
static void processInput(char *);

// Helper functions

// function to initialize the board
static void init_board()
{
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            board[i][j] = '-';
        }
    }
}

// function to check if the board is full
static int isFull()
{
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (board[i][j] == '-')
            {
                return 0; // If any square is still empty, return 0 (false)
            }
        }
    }
    continue_game = false;
    message = MSG_GAME_OVER_TIE;
    return 1; // If all squares are filled, print the message and return 1 (true)
}

static void WhoWon(char player)
{
    if (player == 'X')
    {
        message = MSG_PLAYER_WON;
    }
    else
    {
        message = MSG_AI_WON;
    }
}

static int checkWin(char player)
{
    int i;
    // check rows
    for (i = 0; i < 3; i++)
    {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
        {
            WhoWon(player);
            continue_game = false;
            return 1;
        }
    }
    // check columns
    for (i = 0; i < 3; i++)
    {
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player)
        {
            WhoWon(player);
            continue_game = false;

            return 1;
        }
    }
    // check diagonals
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
    {
        WhoWon(player);
        continue_game = false;

        return 1;
    }
    // check other diagonal
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
    {
        WhoWon(player);
        continue_game = false;
        return 1;
    }
    return 0;
}

// function to check if the move is valid
static int validMove(int row, int col)
{

    // check if the row and column are within the board
    if (row < 0 || row > 2 || col < 0 || col > 2)
    {
        if (IsAiTurn)
        {
            return 0;
        }
        message = MSG_INVALID_MOVE;
        continue_game = false;

        return 0;
    }
    // check if the position is already occupied
    if (board[row][col] != '-')
    {
        if (IsAiTurn)
        {
            return 0;
        }
        message = MSG_INVALID_MOVE;
        continue_game = false;

        return 0;
    }
    return 1;
}

// function to reset the board
static void resetBoard()
{
    init_board();
    message = MSG_RESET;
}

// function to make a move
static void makeMove(int row, char col, char player)
{
    int colIndex = -1;
    row = row - 1;
    switch (col)
    {
    case 'A':
    case 'a':
        colIndex = 0;
        break;
    case 'B':
    case 'b':
        colIndex = 1;
        break;
    case 'C':
    case 'c':
        colIndex = 2;
        break;
    default:
        continue_game = false;
        message = MSG_INVALID_MOVE;
        return;
    }
    if (validMove(row, colIndex))
    {
        board[row][colIndex] = player;
        message = MSG_BOARD;
        continue_game = true;
    }
}

// ai function to make a move for the computer
static void aiMove()
{
    int row, col;
    // set boolean to true
    IsAiTurn = true;
    // generate random numbers for row and column until a valid move is found
    do
    {
        get_random_bytes(&row, sizeof(row));
        get_random_bytes(&col, sizeof(col));
        row = row % 3;
        col = col % 3;
    } while (!validMove(row, col));

    // if the move is valid, make the move
    if (validMove(row, col))
    {
        board[row][col] = 'O';
    }
}

//  function that accepts player input and calls the makeMove function, resets the board, prints the board
static void processInput(char *command)
{
    int row;
    char col;
    char *token;
    // get the input string from the user and store it in the input variable us>
    char *input = kmalloc(strlen(command) + 1, GFP_KERNEL);

    // set boolean to false
    IsAiTurn = false;

    if (!input)
    {
        return;
    }
    strcpy(input, command);

    token = strsep(&input, " ");
    if (token != NULL)
    {
        if (strcmp(token, "TURN") == 0)
        {
            // this token holds the col
            token = strsep(&input, " ");
            if (token != NULL)
            {
                col = token[0]; // Use token[0] to get the first character of the token as a char
                // this token holds the row
                token = strsep(&input, " ");
                if (token != NULL)
                {
                    row = simple_strtol(token, NULL, 10);
                    makeMove(row, col, 'X');
                }
                else
                {
                    message = MSG_INVALID_COMMAND;
                    continue_game = false;
                }
            }
            else
            {
                message = MSG_INVALID_COMMAND;
                continue_game = false;
            }
        }
        else if ((strcmp(token, "RESET\n") == 0) || (strcmp(token, "reset\n") == 0))
        {
            resetBoard();
            continue_game = false;
        }
        else if ((strcmp(token, "BOARD\n") == 0) || (strcmp(token, "board\n") == 0))
        {
            continue_game = false;
            message = MSG_BOARD;
        }
        else
        {
            message = MSG_INVALID_COMMAND;
        }
    }
    else
    {
        message = MSG_INVALID_COMMAND;
    }
    kfree(input); // free the dynamically allocated memory
}

// Called when the device is opened
static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "tictactoe: device_open(%p)\n", file);
    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
int msg_len = 0;
    int board_len;
    char *msg_str;
    if (!access_ok(buffer,length))

        {	printk("its access ok");
                return -EFAULT;
        }

    // check if buffer is null
    if (!buffer)
    {
        return -EINVAL;
    }

    switch (message)
    {
    case MSG_INVALID_MOVE:
        msg_str = (char *)messages[MSG_INVALID_MOVE];
        break;
    case MSG_GAME_OVER_TIE:
        msg_str = (char *)messages[MSG_GAME_OVER_TIE];
        break;
    case MSG_PLAYER_WON:
	msg_str = (char *)messages[MSG_PLAYER_WON];
        break;
    case MSG_RESET:
        msg_str = (char *)messages[MSG_RESET];
        break;
    case MSG_INVALID_COMMAND:
        msg_str = (char *)messages[MSG_INVALID_COMMAND];
        break;
    case MSG_AI_WON:
        msg_str = (char *)messages[MSG_AI_WON];
        break;
    case MSG_BOARD:
    {
        char *board_str;
	printk("in board");
        board_str = kmalloc(100 * sizeof(char), GFP_KERNEL);
        if (!board_str)
        {
            return -ENOMEM;
        }
        board_len = snprintf(board_str, 100, "\n   A   B   C\n1  %c | %c | %c\n  ---|---|---\n2  %c | %c | %c\n  ---|---|---\n3  %c | %c | %c\n\n",
                             board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2]);
        if (board_len >= length)
        {
            kfree(board_str);
            return -EINVAL;
        }
        msg_str = board_str;
        msg_len = board_len;
	printk("it got to board");
        break;
    }
    default:
        return 0;
    }

    msg_len = strlen(msg_str) + 1;
    if (msg_len > length)
    {
        return -EINVAL;
    }


    if (copy_to_user(buffer, msg_str, msg_len))
    {
        return -EFAULT;
    }
   
   //reset message
   message = -1;

   
   kfree(msg_str);
    

   return msg_len;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    // accept commands from the user and update the board accordingly
    char *command;

    // check if buffer is null
    if (buffer == NULL)
    {
        return -EINVAL;
    }
    command = kmalloc(length + 1, GFP_KERNEL); // allocate memory for the command + 1 for the null terminator

    if (!command)
    {
        return -ENOMEM; // out of memory
    }

    if (copy_from_user(command, buffer, length))
    {
        kfree(command);
        return -EFAULT;
    }

    // process the command and perform the appropriate action
    processInput(command);

    // check if game is over after player X's move
    if( checkWin('X') || checkWin('O') || isFull()){
        continue_game=false;
        }


        // check if game is over after player O's move
        if (continue_game)
    {
        // player O's turn
        aiMove();
        // check if game is over after player O's move
        if( checkWin('X') || checkWin('O') || isFull()){
        continue_game=false;
        }

    }

    // free the memory allocated for the command
    kfree(command);

    return length;
}

// Called when the device is released
static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "game: device_release(%p,%p)\n", inode, file);
    return 0;
}

// Device file operations struct
static struct file_operations myfops = {
    .open = device_open,
    .write = device_write,
    .read = device_read,
    .release = device_release};

struct miscdevice my_misc = {

    .minor = MISC_DYNAMIC_MINOR,

    .name = "tictactoe",

    .fops = &myfops,

    .mode = S_IRUGO | S_IWUGO};

// Called when the module is loaded
static int __init game_init(void)
{
    int returnval;
    returnval = misc_register(&my_misc);

    if (returnval != 0)

    {

        printk(KERN_ERR "failed to register game device: %d\n", returnval);

        return returnval;
    }

    // Initialize the board
    init_board();

    printk(KERN_INFO "game device registered: /dev/%s\n", my_misc.name);

    return 0;
}

// Called when the module is unloaded
static void __exit game_exit(void)
{
    printk(KERN_INFO "game: exit\n");
    // Unregister the character device
    misc_deregister(&my_misc);
}

module_init(game_init);
module_exit(game_exit);
