#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>

#define forever while (1)

struct student
{
    char name[64]; /* Имя ученика */
    double mark; /* Средняя оценка */
    unsigned int age; /* Возраст */
    char category; /* Категория */
};

#define VECTOR_DATA_TYPE struct student
#include "vector.h"

/* Сортировка по имени */
static int sort_by_name(const void *first, const void *second)
{
    const struct student *s1 = (const struct student*)(*(const void**)(first));
    const struct student *s2 = (const struct student*)(*(const void**)(second));

    return strcasecmp(s1->name, s2->name);
}

/* Сортировка по оценке */
static int sort_by_mark(const void *first, const void *second)
{
    const struct student *s1 = (const struct student*)(*(const void**)(first));
    const struct student *s2 = (const struct student*)(*(const void**)(second));

    if (s1->mark > s2->mark)
        return 1;
    else if (s1->mark < s2->mark)
        return -1;
    else
        return 0;
}

/* Сортировка по возрасту */
static int sort_by_age(const void *first, const void *second)
{
    const struct student *s1 = (const struct student*)(*(const void**)(first));
    const struct student *s2 = (const struct student*)(*(const void**)(second));

    if (s1->age > s2->age)
        return 1;
    else if (s1->age < s2->age)
        return -1;
    else
        return 0;
}

/**
 * @brief save_to_file - сохраняет в файл элементы динамического массива
 * @param v - указатель на структуру динамического массива
 * @param filename - название сохраняемого файла
 */
static void save_to_file(struct vector *v, const char *filename)
{
    if (vector_empty(v))
        return;

    FILE *f = fopen(filename, "w");
    if (!f)
    {
        perror("fopen() failed");
        return;
    }

    fprintf(f, "Список учеников автошколы:\n");
    for (size_t i = 0; i < v->size; ++i)
    {
        struct student *s = vector_at(v, i);
        fprintf(f, "Name: %s; Age: %d; Average mark: %.2f; Category: %c\n", s->name, s->age, s->mark, s->category);
    }

    fclose(f);
}

/**
 * @brief print_vector - выводит все эелементы динамического массива на консоль
 * @param v - указатель на структуру динамического массива
 */
static void print_vector(struct vector *v)
{
    for (size_t i = 0; i < v->size; ++i)
    {
        struct student *s = vector_at(v, i);
        printf("Name: %s, Age: %d, Mark: %f, Category: %c\n", s->name, s->age, s->mark, s->category);
    }
}

/**
 * @brief print_help - печать на экран сообщения о порядке использования программы
 */
static void print_help()
{
    static const char *help = "Exit from program \":q\"\n"
                              "Help: \":h\"\n"
                              "Add new student: \":a {Name, age, average mark, category}\"\n"
                              "Remove student: \":d [index]\"\n"
                              "Clear list: \":c\"\n"
                              "Save to file: \":f <FILENAME>\"\n"
                              "Print list: \":p\"\n"
                              "Sort: \":s <FIELD>[name|mark|age]\"\n";
    printf("%s\n", help);
}

/**
 * @brief print_greetings - печать на экран сообщения приветствия
 */
static void print_greetings()
{
    const char *greetings = "You are welcomed by the program for collecting and analyzing the data of driving school students.";
    printf("%s\n", greetings);
}

/* Функции взаимодействия с пользователем */
static void action_save_in_file(struct vector *v, char *s)
{
    strtok(s, " "); /* Первый вызов функции, первым аргументов которой является введённая строка, для перемещения внуреннего статического указателя строки на вторую подстроку (man strtok) */
    char *filename = strtok(NULL, " "); /* Получение второй подстроки, которая является предполагаемым файлом для записи */

    /* Поиск символа перехода на новую строку, чтобы он не включался в выходное название файла */
    char *newline = strchr(filename, '\n');
    if (newline)
        *newline = '\0';

    if (!filename || strlen(filename) < 1)
    {
        printf("Wrong filename\n");
    }
    else
    {
        save_to_file(v, filename);
        printf("File has been saved successfully in \"%s\".\n", filename);
    }
}

static void action_add_new_student(struct vector *v, char *arg)
{
    char input[256] = {0};
    char *s = NULL;
    int args_count = 0;

    char *tokens[4] = {NULL, NULL, NULL, NULL}; /* Указатели на 4 строки, характеризующие 4 поля структуры ученика */
    s = strtok(arg, " "); /* Первый вызов функции, первым аргументов которой является введённая строка, для перемещения внуреннего статического указателя строки на вторую подстроку (man strtok) */
    if (s != NULL)
    {
        while(1)
        {
            s = strtok(NULL, " "); /* Перемещение по введённой строке и подсчёт количества подстрок, разделённых пробелом */
            if (s == NULL || args_count >= 4)
                break;

            tokens[args_count++] = s;
        }
    }

    struct student st;
    memset(&st, 0, sizeof(struct student));

    int correct_input = 0;
    if (args_count == 4)
    {
        char *endptr = NULL;
        unsigned int age = strtoul(tokens[1], &endptr, 10); /* Конвертирование строки в число */
        if (endptr != tokens[1]) /* Если endptr равен началу строки, значит конвертация в число не удалась */
        {
            double mark = strtod(tokens[2], &endptr);
            if (endptr != tokens[2])
            {
                char c = *tokens[3]; /* Первый символ в подстроке пусть является категорией */
                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) /* Защита от присваивания категории символа перехода на новую строку */
                {
                    correct_input = 1;

                    unsigned int len = strlen(tokens[0]);
                    unsigned int max = len >= sizeof(st.name) ? sizeof(st.name) - 1 : len; /* Нельзя допустить переполнения буфера имени структуры студента */

                    strncpy(st.name, tokens[0], max);
                    st.mark = mark;
                    st.age = age;
                    st.category = c;
                }
            }
        }
    }

    /* Некорретный ввод всех 4 необходимых полей сразу, поэтому поэтапный ввод */
    if (!correct_input)
    {
        printf("Enter name:\n");
        printf("> ");
        s = fgets(input, sizeof(input) - 1, stdin);
        if (!s)
        {
            printf("Incorrect input\n");
            return;
        }
        else
        {
            /* Поиск и удаление введённого символа перевода на новую строку */
            int len = strlen(input), index = len;
            for (int i = len - 1; i >= 0; --i)
            {
                if (input[i] == '\n')
                {
                    index = i;
                    break;
                }
            }

            strncpy(st.name, input, index);
        }

        printf("Enter age:\n");
        printf("> ");
        fscanf(stdin, "%u", &st.age);

        printf("Enter average mark:\n");
        printf("> ");
        fscanf(stdin, "%lf", &st.mark);

        getchar(); /* Очистка буфера ввода от символа перевода на новую строку */
        printf("Enter category:\n");
        printf("> ");
        fscanf(stdin, "%c", &st.category);
        getchar(); /* Очистка буфера ввода от символа перевода на новую строку */
    }

    vector_push_back(v, &st);
    printf("Student added successfully.\n");
}

static void action_sort(struct vector *v, char *s)
{
    if (vector_empty(v))
    {
        printf("Empty list\n");
        return;
    }

    strtok(s, " "); /* Первый вызов функции, первым аргументов которой является введённая строка, для перемещения внуреннего статического указателя строки на вторую подстроку (man strtok) */
    char *field = strtok(NULL, " "); /* Получение подстроки, указывающую по какому полю необходимо сортировать */

    if (!field)
    {
        printf("Incorrect input\n");
        return;
    }

    if (strncmp(field, "name", 4) == 0)
    {
        qsort(vector_data(v), vector_size(v), sizeof(void**), sort_by_name);
        printf("List has been successfully sorted by name\n");
    }
    else if (strncmp(field, "mark", 4) == 0)
    {
        qsort(vector_data(v), vector_size(v), sizeof(void**), sort_by_mark);
        printf("List has been successfully sorted by mark\n");
    }
    else if (strncmp(field, "age", 3) == 0)
    {
        qsort(vector_data(v), vector_size(v), sizeof(void**), sort_by_age);
        printf("List has been successfully sorted by age\n");
    }
    else
    {
        printf("Incorrect input\n");
    }
}

static void action_erase(struct vector *v, char *input)
{
    strtok(input, " "); /* Первый вызов функции, первым аргументов которой является введённая строка, для перемещения внуреннего статического указателя строки на вторую подстроку (man strtok) */
    char *s = strtok(NULL, " ");

    if (!s)
    {
        printf("Incorrect input\n");
        return;
    }

    char *endptr = NULL;
    unsigned int index = strtoul(s, &endptr, 10); /* Перевод строки в число. Endptr указывает на последний несконвертированный символ */
    if (endptr != s)
    {
        if (index < vector_size(v))
            vector_erase(v, index);
        else
            printf("Index (%u) >= size (%lu)\n", index, vector_size(v));
    }
    else
    {
        printf("Index must be a digit\n");
    }
}

int main()
{
    setlocale(LC_ALL, "russian");

    print_greetings();
    print_help();

    /* Создание структуры динамического массива. Размер данных передаётся равными нулю,
     * так как перед включением заголовочного файла vector.h было объявлено макрооопределение с пользовательским типом */
    struct vector *v = vector_create(0);

    char input[256] = {0}; /* Массив для сохранения введённой строки */
    forever /* Бесконечный цикл до ввода пользователем комманд :q или exit */
    {
        printf("> ");
        char *s = fgets(input, sizeof(input) - 1, stdin);

        if (!s || strncmp(s, ":q", 2) == 0 || strncmp(s, "exit", 4) == 0)
        {
            break;
        }
        else if (strncmp(s, ":h", 2) == 0)
        {
            print_help();
        }
        else if (strncmp(s, ":f", 2) == 0)
        {
            action_save_in_file(v, s);
        }
        else if (strncmp(s, ":a", 2) == 0)
        {
            action_add_new_student(v, s);
        }
        else if (strncmp(s, ":p", 2) == 0)
        {
            print_vector(v);
        }
        else if (strncmp(s, ":s", 2) == 0)
        {
            action_sort(v, s);
        }
        else if (strncmp(s, ":c", 2) == 0)
        {
            vector_clear(v);
        }
        else if (strncmp(s, ":d", 2) == 0)
        {
            action_erase(v, s);
        }
        else
        {
            print_help();
        }
    }

    /* Удаление динамического массива */
    vector_destroy(&v);

    return EXIT_SUCCESS;
}
