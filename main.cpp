#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

using namespace std;

const unsigned int MAX_CHAR_ARR_LENGTH = 128;

struct User
{
    // First name
    char fname[MAX_CHAR_ARR_LENGTH];
    // Last name
    char lname[MAX_CHAR_ARR_LENGTH];
    // Username (unique among all users (studnets, professors and manager))
    char username[MAX_CHAR_ARR_LENGTH];
    // Password (Must be at least 8 characters)
    char password[MAX_CHAR_ARR_LENGTH];
    // User role
    // * 'M' => Manager
    // * 'P' => Professor
    // * 'S' => Student
    char role;
} loggedin_user;

struct Exam
{
    // Exam ID is a randomly generated unique number
    char id[MAX_CHAR_ARR_LENGTH];
    // Exam name e.g. "Fundamental Programming Semester Exam"
    char name[MAX_CHAR_ARR_LENGTH];
    // Username of the creator (which is the professor who created it)
    // * The results of this exam will only be available to the corresponding professor
    char creator_username[MAX_CHAR_ARR_LENGTH];
    // Number of questions in the exam
    unsigned int qcount;
    // Start time of the exam in seconds since 00:00 hours, Jan 1, 1970 UTC (UNIX Timestamp)
    time_t start_time;
    // End time of the exam in seconds since 00:00 hours, Jan 1, 1970 UTC (UNIX Timestamp)
    time_t end_time;
};

struct Question
{
    /* true: the question is a multiple choice question
     * will be asked for 4 options (Question::opt1, ..., Question::opt4)
     * and a correct option (Question::correct)
     * false: the question is an essay question
     * the answer to these type of questions will be stored in Answer::essay_answer
     */
    bool is_multiple_choice;
    // Question number
    unsigned int qnum;
    // The question itself
    char question[MAX_CHAR_ARR_LENGTH];
    // Four options for multiple choice questions
    char opt1[MAX_CHAR_ARR_LENGTH];
    char opt2[MAX_CHAR_ARR_LENGTH];
    char opt3[MAX_CHAR_ARR_LENGTH];
    char opt4[MAX_CHAR_ARR_LENGTH];
    /* The correct option character
     * 'a', 'b', 'c', 'd'
     */
    char correct;
};

struct Answer
{
    // Exam ID of the exam which this answer is related to
    char exam_id[MAX_CHAR_ARR_LENGTH];
    // Username of the student whome this answer belongs to
    char username[MAX_CHAR_ARR_LENGTH];
    // Question number corresponding to this answer
    unsigned int qnum;
    // To determine if this answer is an answer to a multiple choice question or not
    bool is_multiple_choice;
    // Option choosed by the student if is_multiple_choise is true
    char chosen;
    // Answer to an essay question if is_multiple_choise is false
    char essay_answer[512];
};

struct Result
{
    // Username of the student whome this result belongs to
    char username[MAX_CHAR_ARR_LENGTH];
    // ID of an exam which this result belongs to
    char exam_id[MAX_CHAR_ARR_LENGTH];
    // Name of the exam which this result belongs to
    char exam_name[MAX_CHAR_ARR_LENGTH];
    // Number of correctly answered multiple choice questions
    int correct_choices_count;
    // Number of wrongly answered multiple choice questions
    int wrong_choices_count;
    // Total number of multiple choice questions
    int multiple_choice_count;
    /* Calculated score in percent
     * Each question has 3 scores 
     * 3 scores is given for a correct answer
     * 1 score is taken for a wrong answer
     * 0 score for a blank answer
     */
    float multiple_choice_percent;
    /* The time which the result will be visible by a user
     * Which is the end time of the corresponding exam (Exam::end_time)
    */
    time_t visible_time;
};

void on_startup();
void setup();
void show_main_menu();
int register_user(User *user);
int add_exam(Exam *exam);
void take_exam();
void show_exam_results_P();
void show_exam_results_S();
void show_exam_answers();
void get_datetime_input(tm *datetime);
void welcome_user(User *user);
bool login_screen();
bool validate_password(char *pass, char *pass_repeat);
bool username_exists(char *username);
bool rand_id_exists(char *rand_id);
void list_all_users(char user_role);
void list_all_exams();
void read_input(char *var);
void clear_console();
void wait_on_enter();
void print_date(tm *timedate);
void print_time(tm *timedate);
void print_fullname_of_username(char *username);
void logout(User loggedin_user);
void create_examQ_path(char *exam_path, char *exam_id);
void create_examA_path(char *exam_path, char *exam_id);
void create_examR_path(char *exam_path, char *exam_id);

int main()
{
    // Seeding random funciton
    srand(time(NULL));

    on_startup();

    return 0;
}

void on_startup()
{
// Creating ./data directory if it doesn't exist to store the files inside it
#ifdef __unix__
    system("mkdir -p data");
#elif _WIN32
    system("if not exist data mkdir data");
#else
    cout << "Cannot run this program on this OS";
    exit(1);
#endif

    clear_console();
    // Checking if it's the first time the program is being run
    bool is_first_startup;
    FILE *file_ptr;
    file_ptr = fopen("./data/is_first_startup.dat", "a+");
    if (file_ptr == NULL)
    {
        cout << "*** Error: Make sure you have enough disk space, and the "
                "permission to read from or write into files. ***\n";
        cout << "Exiting EMS...";
        exit(0);
    }
    size_t data_read_s = fread(&is_first_startup, sizeof(is_first_startup), 1, file_ptr);
    fclose(file_ptr);
    if (is_first_startup || data_read_s == 0)
    {
        setup();
        is_first_startup = false;
        file_ptr = fopen("./data/is_first_startup.dat", "a+");
        fwrite(&is_first_startup, sizeof(is_first_startup), 1, file_ptr);
        fclose(file_ptr);
    }

    // Loops until valid login
    while (!login_screen())
    {
        cout << "\t*** Error: The username or password you entered is incorrect. "
                "Make sure you are registered as a user. ***\n";
        wait_on_enter();
    }

    // Successful login message
    cout << "\tSuccess: You've logged in as " << loggedin_user.fname << " " << loggedin_user.lname;
    cout << " (";
    if (loggedin_user.role == 'M')
        cout << "Manager";
    else if (loggedin_user.role == 'P')
        cout << "Professor";
    else if (loggedin_user.role == 'S')
        cout << "Student";
    else
        cout << "Undefined";
    cout << ").\n";
    wait_on_enter();

    show_main_menu();
}

void setup()
{
    cout << "########## Welcome to EMS! ##########\n";
    cout << "Just because you are the first one running this program, you are "
            "the one and only manager!\n";

    // Creating needed files
    FILE *file_ptr;
    file_ptr = fopen("./data/users.dat", "w");
    fclose(file_ptr);
    file_ptr = fopen("./data/exams.dat", "w");
    fclose(file_ptr);

    // First user using EMS is counted as the manager
    loggedin_user.role = 'M';
    if (register_user(&loggedin_user))
    {
        cout << "Registrion not successful, Exiting...\n";
        exit(0);
    }
}

bool login_screen()
{
    char username[MAX_CHAR_ARR_LENGTH], password[MAX_CHAR_ARR_LENGTH];
    User user_tmp;
    FILE *file_ptr;

    clear_console();

    cout << "##### Login #####\n";
    cout << "\tUsername: ";
    read_input(username);
    cout << "\tPassword: ";
    read_input(password);

    // Checking if the login is valid (username exists and password is correct)
    file_ptr = fopen("./data/users.dat", "r");
    fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
    while (!feof(file_ptr))
    {
        if (strcmp(user_tmp.username, username) == 0)
            if (strcmp(user_tmp.password, password) == 0)
            {
                loggedin_user = user_tmp;
                break;
            }
        fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
    }
    int is_eof = feof(file_ptr);
    fclose(file_ptr);
    if (is_eof)
        return false;
    else
        return true;
}

void show_main_menu()
{
    char menu_code;

    clear_console();
    welcome_user(&loggedin_user);

    cout << "\t(0) Refresh\n";
    // Rendering different menus depending on the user role
    switch (loggedin_user.role)
    {
    // Manager Menu
    case 'M':
        cout << "\t(1) List all students\n";
        cout << "\t(2) List all professors\n";
        cout << "\t(3) List all exams\n";
        cout << "\t(4) Add a new user\n";
        cout << "\t(5) Logout\n";

        cout << "\nEnter menu option code: ";
        cin >> menu_code;
        cin.ignore();

        switch (menu_code)
        {
        case '0':
            break;
        case '1':
            list_all_users('S');
            break;
        case '2':
            list_all_users('P');
            break;
        case '3':
            list_all_exams();
            break;
        case '4':
            User new_user;
            /* Fixes the bug that newly created User struct equals to loggedin user
             * Initializing user role to 'X' so the role selection will appear in register_user() function
             */
            new_user.role = 'X';
            register_user(&new_user);
            break;
        case '5':
            logout(loggedin_user);
            break;
        default:
            cout << "\t*** Error: Undefined menu code. Try again. ***\n";
            wait_on_enter();
        }
        break;
    // Professor Menu
    case 'P':
        cout << "\t(1) List all students\n";
        cout << "\t(2) List all exams\n";
        cout << "\t(3) See exam results\n";
        cout << "\t(4) See student answers\n";
        cout << "\t(5) Add a new exam\n";
        cout << "\t(6) Logout\n";

        cout << "\nEnter menu option code: ";
        cin >> menu_code;
        cin.ignore();

        switch (menu_code)
        {
        case '0':
            break;
        case '1':
            list_all_users('S');
            break;
        case '2':
            list_all_exams();
            break;
        case '3':
            show_exam_results_P();
            break;
        case '4':
            show_exam_answers();
            break;
        case '5':
            Exam new_exam;
            add_exam(&new_exam);
            break;
        case '6':
            logout(loggedin_user);
            break;
        default:
            cout << "\t*** Error: Undefined menu code. Try again. ***\n";
            wait_on_enter();
        }
        break;
    // Student Menu
    case 'S':
        cout << "\t(1) List all exams\n";
        cout << "\t(2) Take an exam\n";
        cout << "\t(3) Exam results\n";
        cout << "\t(4) Exam answers\n";
        cout << "\t(5) Logout\n";

        cout << "\nEnter menu option code: ";
        cin >> menu_code;
        cin.ignore();

        switch (menu_code)
        {
        case '0':
            break;
        case '1':
            list_all_exams();
            break;
        case '2':
            take_exam();
            break;
        case '3':
            show_exam_results_S();
            break;
        case '4':
            show_exam_answers();
            break;
        case '5':
            logout(loggedin_user);
            break;
        default:
            cout << "\t*** Error: Undefined menu code. Try again. ***\n";
            wait_on_enter();
        }
        break;
    default:
        cout << "\t*** Undefined role. ***\n";
        exit(0);
    }

    show_main_menu();
}

void welcome_user(User *user)
{
    time_t curr_time = time(NULL);
    cout << "##########";
    cout << " Welcome to EMS! ";
    cout << user->fname << " " << user->lname << " ";
    cout << "(Role: ";
    if (user->role == 'M')
        cout << "Manager";
    else if (user->role == 'P')
        cout << "Professor";
    else if (user->role == 'S')
        cout << "Student";
    else
        cout << "Undefined";
    cout << ") ";
    cout << "##########\n";
    tm *now_timedate = localtime(&curr_time);
    cout << "Today: ";
    print_date(now_timedate);
    cout << ' ';
    print_time(now_timedate);
    cout << '\n';
}

int register_user(User *user)
{
    cout << "Registring new user\n";
    char password[MAX_CHAR_ARR_LENGTH];
    char password_repeat[MAX_CHAR_ARR_LENGTH];
    if (user->role != 'M')
    {
        cout << "\tEnter the role of the user [(S)tudent|(P)rofessor]: ";
        cin >> user->role;
        cin.ignore();
        while (user->role != 'S' && user->role != 'P')
        {
            cout << "\t*** Error: The entered role was undefined, please enter "
                    "either S or P ***\n";
            cin >> user->role;
            cin.ignore();
        }
    }
    cout << "\tFirst name: ";
    read_input(user->fname);
    // Captalizing first name
    if (user->fname[0] >= 'a' && user->fname[0] <= 'z') user->fname[0] -= 'a' - 'A';
    cout << "\tLast name: ";
    read_input(user->lname);
    // Captalizing last name
    if (user->lname[0] >= 'a' && user->lname[0] <= 'z') user->lname[0] -= 'a' - 'A';

    // Checking if username is already taken by other users
    cout << "\tUsername: ";
    read_input(user->username);
    while (username_exists(user->username))
    {
        cout << "\t*** Error: username exists, enter another one. ***\n";
        cout << "\tUsername: ";
        read_input(user->username);
    }

    cout << "\tCreate a password: ";
    read_input(password);
    cout << "\tRepeat the password: ";
    read_input(password_repeat);
    while (!validate_password(password, password_repeat))
    {
        cout << "\tTry again!\n";
        cout << "\tCreate a password: ";
        read_input(password);
        cout << "\tRepeat the password: ";
        read_input(password_repeat);
    }
    strcpy(user->password, password);
    FILE *file_ptr;

    if (user->role == 'S')
    {
        // Creating file for the exam mapping
        char map_exam_file_path[MAX_CHAR_ARR_LENGTH] = "./data/map_";
        strcat(map_exam_file_path, user->username);
        strcat(map_exam_file_path, ".dat");
        file_ptr = fopen(map_exam_file_path, "w");
        fclose(file_ptr);
    }

    // Saving User struct
    file_ptr = fopen("./data/users.dat", "a");
    size_t data_written_s = fwrite(user, sizeof(User), 1, file_ptr);
    fclose(file_ptr);

    if (data_written_s == 1)
    {
        cout << '\t' << user->fname << ' ' << user->lname
             << " has been successfully registered as ";
        if (user->role == 'M')
            cout << "the manager";
        else if (user->role == 'P')
            cout << "a professor";
        else if (user->role == 'S')
            cout << "a student";
        else
        {
            cout << "*** Internal Error: Undefined role for user ***\n";
            exit(0);
        }
        cout << ".\n";
        wait_on_enter();
        return 0;
    }
    else
    {
        cout
            << "\t*** Could not save user info, check for file permissions and "
               "disk space and then try again. ***\n";
        wait_on_enter();
        return -1;
    }
}

int add_exam(Exam *new_exam)
{
    tm exam_time;
    cout << "\tEnter the name of the exam: ";
    read_input(new_exam->name);
    cout << "\tHow many questions does the exam have? ";
    cin >> new_exam->qcount;
    cin.ignore();
    while (new_exam->qcount <= 0)
    {
        cout << "\t*** Error: The number of questions cannot be less than 1 ***\n";
        cout << "\tHow many questions does the exam have? ";
        cin >> new_exam->qcount;
        cin.ignore();
    }
    while (true)
    {
        cin.sync();
        time_t time_now;
        cout << "\tWhen does the exam start? (YYYY-MM-DD hh:mm:ss):\n\t";
        get_datetime_input(&exam_time);
        new_exam->start_time = mktime(&exam_time);
        time(&time_now);
        if (new_exam->start_time < time_now)
        {
            cout << "\t*** Error: Exam start time cannot be earlier than this moment. *** \n";
            continue;
        }
        cout << "\tWhen does the exam end? (YYYY-MM-DD hh:mm:ss):\n\t";
        get_datetime_input(&exam_time);
        new_exam->end_time = mktime(&exam_time);
        if (new_exam->end_time <= new_exam->start_time)
        {
            cout << "\t*** Error: The end time must be later than the start time! "
                    "Try again. ***\n";
            continue;
        }
        break;
    }

    int random_id = rand();
    // Converting int to c-style string
    strcpy(new_exam->id, to_string(random_id).c_str());
    while (rand_id_exists(new_exam->id))
    {
        random_id = rand();
        strcpy(new_exam->id, to_string(random_id).c_str());
    }

    strcpy(new_exam->creator_username, loggedin_user.username);

    FILE *file_ptr = fopen("./data/exams.dat", "a+");
    if (file_ptr == NULL)
    {
        cout << "\t*** Error: Couldn't save exam data. Check read/write "
                "permissions and disk space and then try again. ***\n";
        exit(0);
    }
    size_t written_size = fwrite(new_exam, sizeof(Exam), 1, file_ptr);
    if (!written_size)
    {
        cout << "\t*** Error: Couldn't save exam data. Check read/write "
                "permissions and disk space and then try again. ***\n";
        wait_on_enter();
        fclose(file_ptr);
        return -1;
    }

    cout << "\tNew exam has been successfully added!\n";
    wait_on_enter();
    fclose(file_ptr);

    // Creating the path of exam results file (./data/exam_[random_num]_results.dat)
    char exam_results_path[MAX_CHAR_ARR_LENGTH];
    create_examR_path(exam_results_path, new_exam->id);
    FILE *file_temp = fopen(exam_results_path, "w");
    fclose(file_temp);

    // Creating the path of exam answers file (./data/exam_[random_num]_answers.dat)
    char exam_answers_path[MAX_CHAR_ARR_LENGTH];
    create_examA_path(exam_answers_path, new_exam->id);
    file_temp = fopen(exam_answers_path, "w");
    fclose(file_temp);

    // Creating the path of exam questions file (./data/exam_[random_num]_questions.dat)
    char exam_questions_path[MAX_CHAR_ARR_LENGTH];
    create_examQ_path(exam_questions_path, new_exam->id);
    FILE *exam_file = fopen(exam_questions_path, "w+");
    Question new_question;

    cout << "\tAdding the questions:\n";
    for (int i = 0; i < new_exam->qcount; i++)
    {
        new_question.qnum = i + 1;
        cout << "------------------------------------------\n";
        cout << "\tEnter the question #" << new_question.qnum << ": ";
        read_input(new_question.question);

        char user_input;
        while (true)
        {
            cout << "\tIs this question a multiple choice question? (y/n) ";
            cin >> user_input;
            cin.ignore();
            if (user_input == 'n' || user_input == 'N')
                new_question.is_multiple_choice = false;
            else if (user_input = 'y' || user_input == 'Y')
                new_question.is_multiple_choice = true;
            else
            {
                cout << "\t*** Error: Invalid input, enter either y or n ***\n";
                continue;
            }
            break;
        }

        if (new_question.is_multiple_choice)
        {
            cout << "\tEnter the options:\n";
            cout << "\ta) ";
            read_input(new_question.opt1);
            cout << "\tb) ";
            read_input(new_question.opt2);
            cout << "\tc) ";
            read_input(new_question.opt3);
            cout << "\td) ";
            read_input(new_question.opt4);
            while (true)
            {
                cout << "\tWhich option is the correct answer? (a-d) ";
                cin >> new_question.correct;
                cin.ignore();
                if (!(new_question.correct == 'a' ||
                      new_question.correct == 'b' ||
                      new_question.correct == 'c' ||
                      new_question.correct == 'd'))
                {
                    cout << "\t*** Error: Invalid option, please enter a, b, c or d ***\n";
                    continue;
                }
                break;
            }
        }

        size_t data_written = fwrite(&new_question, sizeof(Question), 1, exam_file);
        if (data_written)
            cout << "\tQuestion #" << new_question.qnum << " successfully added.\n";
        else
        {
            cout << "\t*** Error: Failed to save the question! "
                    "check the write permission for files in this directory and then try again. ***\n";
            i--;
        }
    }
    fclose(exam_file);

    cout << "\tExam questions successfully added.\n";
    wait_on_enter();

    return 0;
}

void take_exam()
{
    char exam_id[MAX_CHAR_ARR_LENGTH];
    cout << "\tEnter the exam id you want to take: ";
    read_input(exam_id);
    Exam this_exam;
    FILE *exam_file = fopen("./data/exams.dat", "r");
    time_t time_now;
    fread(&this_exam, sizeof(Exam), 1, exam_file);
    while (!feof(exam_file))
    {
        if (strcmp(this_exam.id, exam_id) == 0) break;
        fread(&this_exam, sizeof(Exam), 1, exam_file);
    }
    if (feof(exam_file))
    {
        cout << "\t*** Error: No exam found with this id. ***\n";
        wait_on_enter();
        return;
    }
    fclose(exam_file);

    char user_response;
    cout << "\tExam found.\n";
    while (true)
    {
        cout << "\tIs \"" << this_exam.name << "\" the right exam? (y/n) ";
        cin >> user_response;
        cin.ignore();
        if (user_response == 'n' || user_response == 'N')
            return;
        else if (user_response == 'y' || user_response == 'Y')
        {
            // Checking if the user has already taken the exam or not
            char exam_map_file_path[MAX_CHAR_ARR_LENGTH] = "./data/map_";
            char exam_id[sizeof(Exam::id)];
            strcat(exam_map_file_path, loggedin_user.username);
            strcat(exam_map_file_path, ".dat");
            FILE *student_exam_map = fopen(exam_map_file_path, "r");
            fread(&exam_id, sizeof(exam_id), 1, student_exam_map);
            while (!feof(student_exam_map))
            {
                if (strcmp(exam_id, this_exam.id) == 0)
                {
                    cout << "You have already taken this exam.\n";
                    wait_on_enter();
                    return;
                }
                fread(&exam_id, sizeof(exam_id), 1, student_exam_map);
            }
            time(&time_now);
            if (time_now >= this_exam.start_time && time_now < this_exam.end_time)
                break;
            else if (time_now < this_exam.start_time)
            {
                cout << "The exam has not started yet. It starts on ";
                print_date(localtime(&this_exam.start_time));
                cout << ' ';
                print_time(localtime(&this_exam.start_time));
                cout << '\n';
                wait_on_enter();
                return;
            }
            else
            {
                cout << "The exam has been ended. It was ended on ";
                print_date(localtime(&this_exam.end_time));
                cout << ' ';
                print_time(localtime(&this_exam.end_time));
                cout << '\n';
                wait_on_enter();
                return;
            }
        }
        else
            cout << "\tInvalid input.\n";
        continue;
    }

    clear_console();
    cout << this_exam.name;
    cout << ": Loading questions...\n";

    Question exam_question;
    Answer user_answer;
    Result user_results;

    strcpy(user_results.username, loggedin_user.username);
    strcpy(user_results.exam_id, this_exam.id);
    strcpy(user_results.exam_name, this_exam.name);
    user_results.visible_time = this_exam.end_time;
    user_results.correct_choices_count = 0;
    user_results.wrong_choices_count = 0;
    user_results.multiple_choice_count = 0;

    char questions_path[MAX_CHAR_ARR_LENGTH];
    create_examQ_path(questions_path, this_exam.id);

    char answers_path[MAX_CHAR_ARR_LENGTH];
    create_examA_path(answers_path, this_exam.id);

    FILE *examQ_file = fopen(questions_path, "r");
    FILE *examA_file = fopen(answers_path, "a+");

    wait_on_enter();
    fread(&exam_question, sizeof(Question), 1, examQ_file);
    time(&time_now);
    while (!feof(examQ_file) && time_now <= this_exam.end_time)
    {
        strcpy(user_answer.username, loggedin_user.username);
        strcpy(user_answer.exam_id, this_exam.id);

        time_t remaining = this_exam.end_time - time_now;

        cout << "-------------------------------------------------\n";

        cout << "Remaning Time: ";
        print_time(gmtime(&remaining));
        cout << '\n';

        cout << "(" << exam_question.qnum << ") " << exam_question.question << '\n';
        user_answer.qnum = exam_question.qnum;
        if (exam_question.is_multiple_choice)
        {
            cout << "a) " << exam_question.opt1 << '\n';
            cout << "b) " << exam_question.opt2 << '\n';
            cout << "c) " << exam_question.opt3 << '\n';
            cout << "d) " << exam_question.opt4 << '\n';
            time(&time_now);
            while (this_exam.end_time > time_now)
            {
                cout << "Enter your choice (a-d) or enter x for blank: ";
                cin >> user_answer.chosen;
                cin.ignore();
                if (!(user_answer.chosen == 'a' ||
                      user_answer.chosen == 'b' ||
                      user_answer.chosen == 'c' ||
                      user_answer.chosen == 'd' ||
                      user_answer.chosen == 'x'))
                {
                    cout << "*** Error: Invalid input. Choices are: a, b, c or d or x for blank. ***\n";
                    continue;
                }
                break;
            }
            time(&time_now);
            if (time_now > this_exam.end_time)
            {
                cout << "Exam time is over! Your last answer has been saved.\n";
                wait_on_enter();
                break;
            }
            user_answer.is_multiple_choice = true;
            user_results.multiple_choice_count++;
            if (user_answer.chosen == exam_question.correct)
                user_results.correct_choices_count++;
            else if (user_answer.chosen != 'x')
                user_results.wrong_choices_count++;
            time(&time_now);
        }
        else
        {
            user_answer.is_multiple_choice = false;
            cout << "Enter your answer:\n";
            cout << "> ";
            read_input(user_answer.essay_answer);
            time(&time_now);
            if (time_now > this_exam.end_time)
            {
                cout << "Exam time has ended!\n";
                wait_on_enter();
                break;
            }
        }
        fwrite(&user_answer, sizeof(Answer), 1, examA_file);
        fread(&exam_question, sizeof(Question), 1, examQ_file);
        time(&time_now);
    }

    // Calculating multiple choice score ((3*correct - wrong) / (3*total)) * 100
    if (user_results.multiple_choice_count > 0)
    {
        user_results.multiple_choice_percent = (3 * user_results.correct_choices_count) - user_results.wrong_choices_count;
        user_results.multiple_choice_percent /= (3 * user_results.multiple_choice_count);
        user_results.multiple_choice_percent *= 100;
    }
    else
    {
        user_results.multiple_choice_percent = 100;
    }

    char results_path[MAX_CHAR_ARR_LENGTH];
    create_examR_path(results_path, this_exam.id);
    FILE *results_file = fopen(results_path, "a+");
    fwrite(&user_results, sizeof(Result), 1, results_file);
    fclose(results_file);

    char exam_map_file_path[MAX_CHAR_ARR_LENGTH] = "./data/map_";
    strcat(exam_map_file_path, loggedin_user.username);
    strcat(exam_map_file_path, ".dat");
    FILE *student_exam_map = fopen(exam_map_file_path, "a+");
    fwrite(&this_exam.id, sizeof(this_exam.id), 1, student_exam_map);
    fclose(student_exam_map);

    cout << "Exam compeleted.\n";
    wait_on_enter();
    fclose(examQ_file);
    fclose(examA_file);
}

void show_exam_results_P()
{
    char exam_id_to_look_for[MAX_CHAR_ARR_LENGTH];

    char exam_answer_path[MAX_CHAR_ARR_LENGTH]; // Will be the path for exam's Answer structs
    FILE *exam_answer_file;
    Answer student_answer;

    char exam_result_path[MAX_CHAR_ARR_LENGTH]; // Will be the path for exam's Result structs
    FILE *exam_result_file;
    Result student_result;

    FILE *all_exams_file;
    Exam exam_tmp;

    time_t time_now;

    cout << "\tEnter the Exam ID you want to see the results of: ";
    read_input(exam_id_to_look_for);

    all_exams_file = fopen("./data/exams.dat", "r");
    fread(&exam_tmp, sizeof(Exam), 1, all_exams_file);
    while (!feof(all_exams_file))
    {
        if (strcmp(exam_tmp.id, exam_id_to_look_for) == 0)
        {
            // Exam found

            // Checking if the exam creator is the same user as the loggedin professor
            if (strcmp(exam_tmp.creator_username, loggedin_user.username) != 0)
            {
                cout << "\t*** Error: You cannot see the results of this exam, because you're not the creator of it. ***\n";

                fclose(all_exams_file);
                wait_on_enter();
                return;
            }

            // Checking if the exam is over
            time(&time_now);
            if (time_now < exam_tmp.end_time)
            {
                cout << "\t*** Error: The exam is not over yet, wait until ";
                print_date(localtime(&exam_tmp.end_time));
                cout << ' ';
                print_time(localtime(&exam_tmp.end_time));
                cout << " ***\n";

                fclose(all_exams_file);
                wait_on_enter();
                return;
            }

            break;
        }
        fread(&exam_tmp, sizeof(Exam), 1, all_exams_file);
    }
    // Found exam is stored in exam_tmp struct
    // * if the end of file has not been reached
    int is_eof = feof(all_exams_file);
    fclose(all_exams_file);
    if (is_eof)
    {
        cout << "\t*** Error: No exam found with this ID ***\n";
        wait_on_enter();
        return;
    }
    clear_console();
    cout << "Exam Name | Exam ID | Started on | Ended on | Question count\n";
    cout << "------------------------------------------------------------\n";

    // Printing exam details
    cout << exam_tmp.name << " | ";
    cout << exam_tmp.id << " | ";
    print_date(localtime(&exam_tmp.start_time));
    print_time(localtime(&exam_tmp.start_time));
    cout << " | ";
    print_date(localtime(&exam_tmp.end_time));
    print_time(localtime(&exam_tmp.end_time));
    cout << " | ";
    cout << exam_tmp.qcount;
    cout << '\n';

    create_examR_path(exam_result_path, exam_tmp.id);
    exam_result_file = fopen(exam_result_path, "r");

    create_examA_path(exam_answer_path, exam_tmp.id);

    cout << "\tStudent full name | Student username | Correct | Wrong | Total multiple choice | Percentage\n";
    cout << "\t-------------------------------------------------------------------------------------------\n";
    fread(&student_result, sizeof(student_result), 1, exam_result_file);
    while (!feof(exam_result_file))
    {
        cout << '\t';
        print_fullname_of_username(student_result.username);
        cout << " | ";
        cout << student_result.username << " | ";
        cout << student_result.correct_choices_count << " | ";
        cout << student_result.wrong_choices_count << " | ";
        cout << student_result.multiple_choice_count << " | ";
        cout << student_result.multiple_choice_percent;
        cout << '\n';

        exam_answer_file = fopen(exam_answer_path, "r");

        // Printing essay question answers
        fread(&student_answer, sizeof(Answer), 1, exam_answer_file);
        while (!feof(exam_answer_file))
        {
            if (strcmp(student_result.username, student_answer.username) == 0 && !student_answer.is_multiple_choice)
            {
                cout << "\tQuestion #" << student_answer.qnum;
                cout << ":  " << student_answer.essay_answer;
                cout << '\n';
            }
            fread(&student_answer, sizeof(Answer), 1, exam_answer_file);
        }
        fclose(exam_answer_file);

        cout << "\t-------------------------------------------------------------------------------------------\n";
        fread(&student_result, sizeof(student_result), 1, exam_result_file);
    }
    fclose(exam_result_file);

    cout << '\n';
    wait_on_enter();
}

void show_exam_results_S()
{
    // Creating map file path for the loggedin user
    char exam_map_file_path[MAX_CHAR_ARR_LENGTH] = "./data/map_";
    strcat(exam_map_file_path, loggedin_user.username);
    strcat(exam_map_file_path, ".dat");

    FILE *student_exam_map_file;
    char exam_id[sizeof(Exam::id)];

    char exam_result_path[MAX_CHAR_ARR_LENGTH];
    FILE *exam_result_file;
    Result student_result;

    char exam_answer_path[MAX_CHAR_ARR_LENGTH];
    FILE *exam_answer_file;
    Answer student_answer;

    time_t time_now;

    char exam_id_to_look_for[MAX_CHAR_ARR_LENGTH];
    cout << "\tEnter the Exam ID you want to see the results of: ";
    read_input(exam_id_to_look_for);

    // Checking if the student has taken this exam or not
    student_exam_map_file = fopen(exam_map_file_path, "r");
    fread(exam_id, sizeof(exam_id), 1, student_exam_map_file);
    while (!feof(student_exam_map_file))
    {
        if (strcmp(exam_id, exam_id_to_look_for) == 0)
            // Exam found in students map file
            break;

        fread(exam_id, sizeof(exam_id), 1, student_exam_map_file);
    }
    // Found exam ID is stored in exam_id
    // * if the end of file has not been reached
    int is_eof = feof(student_exam_map_file);
    fclose(student_exam_map_file);
    if (is_eof)
    {
        cout << "\t*** Error: You have not taken any exam with this ID ***\n";
        wait_on_enter();
        return;
    }

    // Creating results file path for the found exam
    create_examR_path(exam_result_path, exam_id);
    exam_result_file = fopen(exam_result_path, "r");

    // Creating answer file path for the found exam
    create_examA_path(exam_answer_path, exam_id);
    exam_answer_file = fopen(exam_answer_path, "r");

    fread(&student_result, sizeof(Result), 1, exam_result_file);
    while (!feof(exam_result_file))
    {
        if (strcmp(student_result.username, loggedin_user.username) == 0)
        {
            // Checking if the time has come to show the results
            time(&time_now);
            if (time_now < student_result.visible_time)
            {
                cout << "\t*** Error: You cannot see the results right now, wait until the exam is over ***\n";
                cout << "\tThe results will be available on ";
                print_date(localtime(&student_result.visible_time));
                cout << ' ';
                print_time(localtime(&student_result.visible_time));
                cout << '\n';

                fclose(exam_result_file);
                fclose(exam_answer_file);
                wait_on_enter();
                return;
            }

            // Showing exam result with the ID of exam_id
            cout << "Name | Exam ID | Correct | Wrong | Total Multiple Choice | Percentage\n";
            cout << "---------------------------------------------------------------------\n";

            cout << student_result.exam_name << " | ";
            cout << student_result.exam_id << " | ";
            cout << student_result.correct_choices_count << " | ";
            cout << student_result.wrong_choices_count << " | ";
            cout << student_result.multiple_choice_count << " | ";
            cout << student_result.multiple_choice_percent << '\n';

            // Printing essay question answers
            fread(&student_answer, sizeof(Answer), 1, exam_answer_file);
            while (!feof(exam_answer_file))
            {
                if (strcmp(student_answer.username, loggedin_user.username) == 0 && !student_answer.is_multiple_choice)
                {
                    cout << "\tQuestion #" << student_answer.qnum;
                    cout << ":  " << student_answer.essay_answer;
                    cout << '\n';
                }
                fread(&student_answer, sizeof(Answer), 1, exam_answer_file);
            }
            cout << "---------------------------------------------------------------------\n";

            break;
        }
        fread(&student_result, sizeof(Result), 1, exam_result_file);
    }
    fclose(exam_result_file);
    fclose(exam_answer_file);

    cout << '\n';
    wait_on_enter();
}

void show_exam_answers()
{
    char exam_id_to_show_answers[MAX_CHAR_ARR_LENGTH];
    char username_to_show_answers[MAX_CHAR_ARR_LENGTH];
    FILE *all_exams_file;
    Exam exam_tmp;

    FILE *student_exam_map_file;
    char exam_map_file_path[MAX_CHAR_ARR_LENGTH];
    char exam_id_student_took[MAX_CHAR_ARR_LENGTH];

    FILE *exam_answer_file;
    char exam_answer_path[MAX_CHAR_ARR_LENGTH];
    Answer answer_tmp;

    FILE *exam_question_file;
    char exam_question_path[MAX_CHAR_ARR_LENGTH];
    Question question_tmp;

    time_t time_now;

    cout << "\tEnter the Exam ID: ";
    read_input(exam_id_to_show_answers);

    all_exams_file = fopen("./data/exams.dat", "r");
    fread(&exam_tmp, sizeof(Exam), 1, all_exams_file);
    while (!feof(all_exams_file))
    {
        if (strcmp(exam_tmp.id, exam_id_to_show_answers) == 0)
        {
            // Exam found

            // Checking if the professor has permission to access the answers
            if (loggedin_user.role == 'P' && strcmp(exam_tmp.creator_username, loggedin_user.username) != 0)
            {
                cout << "\t*** Error: You do not have permission to see the answers of this exam ***\n";
                cout << "\tYou can only see the answers of an exam that you created yourself\n";

                fclose(all_exams_file);
                wait_on_enter();
                return;
            }

            if (loggedin_user.role == 'S')
            {
                strcpy(exam_map_file_path, "./data/map_");
                strcat(exam_map_file_path, loggedin_user.username);
                strcat(exam_map_file_path, ".dat");

                // Checking if the student has taken this exam or not
                student_exam_map_file = fopen(exam_map_file_path, "r");
                fread(exam_id_student_took, sizeof(exam_id_student_took), 1, student_exam_map_file);
                while (!feof(student_exam_map_file))
                {
                    if (strcmp(exam_id_student_took, exam_tmp.id) == 0)
                        // Exam found in students map file
                        break;

                    fread(exam_id_student_took, sizeof(exam_id_student_took), 1, student_exam_map_file);
                }
                // Found exam ID is stored in exam_id
                // * if the end of file has not been reached
                int is_eof = feof(student_exam_map_file);
                fclose(student_exam_map_file);
                if (is_eof)
                {
                    cout << "\t*** Error: You have not taken any exam with this ID ***\n";
                    wait_on_enter();
                    return;
                }
            }

            // Checking if the time to show answers has come
            time(&time_now);
            if (time_now < exam_tmp.end_time)
            {
                cout << "\t*** Error: The exam is not over yet, wait until ";
                print_date(localtime(&exam_tmp.end_time));
                cout << ' ';
                print_time(localtime(&exam_tmp.end_time));
                cout << " ***\n";

                fclose(all_exams_file);
                wait_on_enter();
                return;
            }

            break;
        }
        fread(&exam_tmp, sizeof(Exam), 1, all_exams_file);
    }
    int is_eof = feof(all_exams_file);
    fclose(all_exams_file);
    if (is_eof)
    {
        cout << "\t*** Error: No exam found with this id ***\n";

        wait_on_enter();
        return;
    }

    clear_console();

    // Printing Questions
    cout << "Exam questions:\n";
    create_examQ_path(exam_question_path, exam_tmp.id);
    exam_question_file = fopen(exam_question_path, "r");
    fread(&question_tmp, sizeof(Question), 1, exam_question_file);
    while (!feof(exam_question_file))
    {
        cout << "Question #" << question_tmp.qnum << ": ";
        cout << question_tmp.question << '\n';

        cout << "\ta) " << question_tmp.opt1;
        cout << "\tb) " << question_tmp.opt2;
        cout << "\tc) " << question_tmp.opt3;
        cout << "\td) " << question_tmp.opt4;
        cout << "\n\tCorrect choice: " << question_tmp.correct;
        cout << "\n--------------------------------------------------------------\n";

        fread(&question_tmp, sizeof(Question), 1, exam_question_file);
    }
    fclose(exam_question_file);

    // Printing the answers
    cout << '\n';
    cout << "Answers:\n";
    create_examA_path(exam_answer_path, exam_tmp.id);
    exam_answer_file = fopen(exam_answer_path, "r");
    fread(&answer_tmp, sizeof(Answer), 1, exam_answer_file);
    while (!feof(exam_answer_file))
    {
        if (loggedin_user.role == 'S' && strcmp(answer_tmp.username, loggedin_user.username) != 0)
        {
            fread(&answer_tmp, sizeof(Answer), 1, exam_answer_file);
            continue;
        }

        cout << "Full Name: ";
        print_fullname_of_username(answer_tmp.username);
        cout << " | ";
        cout << "Username: " << answer_tmp.username << '\n';

        cout << "\tanswered question #" << answer_tmp.qnum << ": ";
        if (answer_tmp.is_multiple_choice)
            if (answer_tmp.chosen == 'x')
                cout << "[blank]";
            else
                cout << answer_tmp.chosen;
        else
            cout << "\n\t" << answer_tmp.essay_answer;

        cout << "\n--------------------------------------------------------------\n";

        fread(&answer_tmp, sizeof(Answer), 1, exam_answer_file);
    }
    fclose(exam_answer_file);

    cout << '\n';
    wait_on_enter();
}

void get_datetime_input(tm *datetime)
{
    // Gets date and time in YYYY-MM-DD hh:mm:ss format
    while (true)
    {
        cin.sync();
        // cin.ignore(INT_MAX);
        cin >> datetime->tm_year;
        datetime->tm_year -= 1900; // Substracting 1900 because tm::tm_year holds (Year-1900)
        if (cin.get() != '-')
        {
            cout << "\t*** Error: Invalid date format, make sure to enter date as "
                    "YYYY-MM-DD. Try agian. ***\n\t";
            continue;
        }
        cin >> datetime->tm_mon;
        datetime->tm_mon -= 1; // Substracting 1 because tm::tm_mon starts from
                               // 0 (Jan:0, Feb:1, ..., Dec: 11)
        if (cin.get() != '-')
        {
            cout << "\t*** Error: Invalid date format, make sure to enter date as "
                    "YYYY-MM-DD. Try agian. ***\n\t";
            continue;
        }
        cin >> datetime->tm_mday;
        if (cin.get() != ' ')
        {
            cout << "\t*** Error: Invalid date format, make sure to enter date as "
                    "YYYY-MM-DD. Try agian. ***\n\t";
            continue;
        }
        cin >> datetime->tm_hour;
        if (cin.get() != ':')
        {
            cout << "\t*** Error: Invalid date format, make sure to enter date as "
                    "hh:mm:ss. Try agian. ***\n\t";
            continue;
        }
        cin >> datetime->tm_min;
        if (cin.get() != ':')
        {
            cout << "\t*** Error: Invalid date format, make sure to enter date as "
                    "hh:mm:ss. Try agian. ***\n\t";
            continue;
        }
        cin >> datetime->tm_sec;
        cin.ignore();
        datetime->tm_isdst = -1;
        break;
    }
}

bool validate_password(char *pass, char *pass_repeat)
{
    if (strcmp(pass, pass_repeat) != 0)
    {
        cout << "\t*** Error: Passwords do not match. ***\n";
        return false;
    }

    // Password length must be longer that 8 characters
    unsigned int count = 0;
    while (pass[count] != '\0')
        count++;
    if (count < 8)
    {
        cout << "\t*** Error: Password must be longer than 8 characters. ***\n";
        return false;
    }

    return true;
}

bool username_exists(char *username)
{
    User user_tmp;
    FILE *file_ptr = fopen("./data/users.dat", "r");
    fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
    while (!feof(file_ptr))
    {
        if (strcmp(user_tmp.username, username) == 0)
        {
            fclose(file_ptr);
            return true;
        }
        fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
    }
    fclose(file_ptr);
    return false;
}

bool rand_id_exists(char *rand_id)
{
    FILE *file_ptr = fopen("./data/exams.dat", "r");
    Exam exam_tmp;
    fread(&exam_tmp, sizeof(Exam), 1, file_ptr);
    while (!feof(file_ptr))
    {
        if (strcpy(exam_tmp.id, rand_id) == 0)
        {
            fclose(file_ptr);
            return true;
        }
        fread(&exam_tmp, sizeof(Exam), 1, file_ptr);
    }
    fclose(file_ptr);
    return false;
}

void list_all_users(char user_role)
{
    User user_tmp;
    FILE *file_ptr = fopen("./data/users.dat", "r");
    unsigned int user_count = 0;
    fread(&user_tmp, sizeof(User), 1, file_ptr);
    while (!feof(file_ptr))
    {
        if (user_role == user_tmp.role) user_count++;
        fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
    }
    if (user_count == 0)
    {
        cout << "\tNo users found.\n";
        fclose(file_ptr);
    }
    else
    {
        fclose(file_ptr);
        // Reading student structs into an array so we can sort them
        User *all_users = new User[user_count];
        unsigned int counter = 0;
        file_ptr = fopen("./data/users.dat", "r");
        fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
        while (!feof(file_ptr) && counter < user_count)
        {
            if (user_role == user_tmp.role)
                all_users[counter++] = user_tmp;
            fread(&user_tmp, sizeof(user_tmp), 1, file_ptr);
        }
        fclose(file_ptr);

        // Sorting all_users array with insertion sort algorithm
        int j;
        User user_copy;
        for (int i = 1; i < user_count; i++)
        {
            j = i - 1;
            user_copy = all_users[i];
            while (j >= 0 && strcmp(all_users[j].lname, all_users[j + 1].lname) > 0)
            {
                all_users[j + 1] = all_users[j];
                j--;
            }
            all_users[j + 1] = user_copy;
        }

        cout << "First name | Last name (sorted) | Username\n";
        cout << "------------------------------------------\n";
        for (int i = 0; i < user_count; i++)
        {
            cout << all_users[i].fname << " | ";
            cout << all_users[i].lname << " | ";
            cout << all_users[i].username << '\n';
            cout << "------------------------------------------\n";
        }

        delete[] all_users;
    }
    cout << '\n';
    wait_on_enter();
}

void list_all_exams()
{
    FILE *exam_file = fopen("./data/exams.dat", "r");
    Exam exam_tmp;
    unsigned int exam_count = 0;
    fread(&exam_tmp, sizeof(Exam), 1, exam_file);
    while (!feof(exam_file))
    {
        exam_count++;
        fread(&exam_tmp, sizeof(Exam), 1, exam_file);
    }
    fclose(exam_file);
    if (exam_count == 0)
        cout << "\tNo exams found.\n";
    else
    {
        // Reading exam structs into an array so we can sort them
        Exam *all_exams = new Exam[exam_count];
        int counter = 0;
        exam_file = fopen("./data/exams.dat", "r");
        fread(&exam_tmp, sizeof(Exam), 1, exam_file);
        while (counter < exam_count && !feof(exam_file))
        {
            all_exams[counter++] = exam_tmp;
            fread(&exam_tmp, sizeof(Exam), 1, exam_file);
        }
        fclose(exam_file);

        // Sorting all_users array with insertion sort algorithm
        int j;
        Exam exam_copy;
        for (int i = 1; i < exam_count; i++)
        {
            j = i - 1;
            exam_copy = all_exams[i];
            while (j >= 0 && all_exams[j].start_time > all_exams[j + 1].start_time)
            {
                all_exams[j + 1] = all_exams[j];
                j--;
            }
            all_exams[j + 1] = exam_copy;
        }

        if (loggedin_user.role == 'P')
            cout << "Created by me | ";

        cout << "Exam name | State | Start time (sorted) | End time | Duration | ID\n";
        cout << "------------------------------------------------------------------\n";
        for (int i = 0; i < exam_count; i++)
        {
            time_t duration, time_now;

            /*
             * Printing asterisk (*) if exam is created by the loggedin user
             * otherwise printing blank ([space])
            */
            if (loggedin_user.role == 'P')
            {
                if (strcmp(all_exams[i].creator_username, loggedin_user.username) == 0)
                    cout << '*';
                else
                    cout << ' ';
                cout << " | ";
            }

            time(&time_now);
            duration = all_exams[i].end_time - all_exams[i].start_time;
            // Name
            cout << all_exams[i].name << " | ";
            // State
            if (all_exams[i].start_time > time_now)
                cout << "Not started yet";
            else if (all_exams[i].end_time < time_now)
                cout << "Has been ended";
            else
                cout << "Currently ongoing...";
            cout << " | ";
            // Start date and time
            print_date(localtime(&all_exams[i].start_time));
            cout << ' ';
            print_time(localtime(&all_exams[i].start_time));
            cout << " | ";
            // End date and time
            print_date(localtime(&all_exams[i].end_time));
            cout << ' ';
            print_time(localtime(&all_exams[i].end_time));
            cout << " | ";
            // Duration
            print_time(gmtime(&duration));
            cout << " | ";
            // ID
            cout << all_exams[i].id << '\n';
            cout << "------------------------------------------------------------------\n";
        }

        delete[] all_exams;
    }

    cout << '\n';
    wait_on_enter();
}

void read_input(char *var)
{
    // Reads input and then replaces '\n' character with '\0'
    fgets(var, MAX_CHAR_ARR_LENGTH - 2, stdin);
    for (int i = 0; i < MAX_CHAR_ARR_LENGTH; i++)
    {
        if (var[i] == '\n')
        {
            var[i] = '\0';
            break;
        }
    }
}

void clear_console()
{
#ifdef __unix__
    system("clear");
#elif _WIN32
    system("cls");
#else
    cout << "\e[1;1H\e[2J";
#endif
}

void wait_on_enter()
{
    cout << "> Press enter to continue...";
    cin.get();
    cin.sync();
}

void print_date(tm *date)
{
    // Prints date in YYYY-MM-DD format
    // YYYY-
    cout << date->tm_year + 1900 << '-';
    // MM-
    if (date->tm_mon + 1 < 10) cout << '0';
    cout << date->tm_mon + 1 << '-';
    // DD
    if (date->tm_mday < 10) cout << '0';
    cout << date->tm_mday;
}

void print_time(tm *time)
{
    // Prints time in hh:mm:ss format
    if (time->tm_hour < 10) cout << '0';
    cout << time->tm_hour << ':';
    // mm:
    if (time->tm_min < 10) cout << '0';
    cout << time->tm_min << ':';
    // ss
    if (time->tm_sec < 10) cout << '0';
    cout << time->tm_sec;
}

void print_fullname_of_username(char *username)
{
    FILE *users_file = fopen("./data/users.dat", "r");
    User user_tmp;
    fread(&user_tmp, sizeof(User), 1, users_file);
    while (!feof(users_file))
    {
        if (strcmp(user_tmp.username, username) == 0)
        {
            cout << user_tmp.fname << ' ' << user_tmp.lname;
            break;
        }
        fread(&user_tmp, sizeof(User), 1, users_file);
    }
    if (feof(users_file)) cout << "Undefined";
    fclose(users_file);
}

void logout(User loggedin_user)
{
    clear_console();
    char user_response;
    cout << "Are you sure you want yo quit? (y/n) ";
    cin >> user_response;
    cin.ignore();
    if (user_response == 'y' || user_response == 'Y')
    {
        cout << "#### Thank you for using EMS ####\n";
        cout << "Bye " << loggedin_user.fname << "! :)\n";
        exit(0);
    }
}

void create_examQ_path(char *exam_path, char *exam_id)
{
    strcpy(exam_path, "./data/exam_");
    strcat(exam_path, exam_id);
    strcat(exam_path, "_questions.dat");
}

void create_examA_path(char *exam_path, char *exam_id)
{
    strcpy(exam_path, "./data/exam_");
    strcat(exam_path, exam_id);
    strcat(exam_path, "_answers.dat");
}

void create_examR_path(char *exam_path, char *exam_id)
{
    strcpy(exam_path, "./data/exam_");
    strcat(exam_path, exam_id);
    strcat(exam_path, "_results.dat");
}