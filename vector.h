#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

/**
 * @brief Структура, определяющая динамический массив
 */
struct vector
{
    size_t size; /* Количество элементов */
    size_t capacity; /* Объём выделяемой памяти в байтах */

    size_t data_sizeof; /* Размер пользовательских данных */
    void **data; /* Массив указателей на данные */
};

/* Если перед включением этого заголовочного файла было объявлено макроопоределение VECTOR_DATA_TYPE,
 * то используемое макроопределение USED_DATA_TYPE используется для указания пользовательского типа,
 * который хранится в массиве.
 */
#if defined VECTOR_DATA_TYPE
#define USED_DATA_TYPE VECTOR_DATA_TYPE
#else
#define USED_DATA_TYPE void *
#endif

/* Предварительные объявления внутренних используемых функций */
static void vector_realloc(struct vector **v);
static void vector_init_memory(struct vector **v);

// Публичный интерфейс
/**
 * @brief vector_create - выделяет память под структуру, хранящую необходимые данные о динамическом массиве
 * @param data_sizeof - если передан 0, то размер пользовательской структуры вычисляется согласно макроопределению USED_DATA_TYPE.
 * Если не ноль, то значение приравнивается к внутреннему полю data_sizeof структуры динамического массива
 * @return возвращает указатель на структуру динамического массива
 */
struct vector *vector_create(size_t data_sizeof)
{
    struct vector *v = (struct vector*)malloc(sizeof(struct vector));
    if (!v)
    {
        /* Если невозможно выделить память, значит дальше делать нечего - в системе катастрофические проблемы */
        exit(1);
    }

    vector_init_memory(&v);

    if (data_sizeof == 0)
        v->data_sizeof = sizeof(USED_DATA_TYPE);
    else
        v->data_sizeof = data_sizeof;

    return v;
}

/**
 * @brief vector_size - размер динамического массива
 * @param v - указатель на структуру динамического массива
 * @return количество элементов в динамическом массиве
 */
size_t vector_size(struct vector *v)
{
    return v->size;
}

/**
 * @brief vector_capacity - объём выделенной памяти под элементы динамического массива. ДЛя уменьшения реаллокаций выделяется чуть больше памяти, чем необходимо для хранения элементов
 * @param v - указатель на структуру динамического массива
 * @return размер выделенной памяти под элементы динамического массива
 */
size_t vector_capacity(struct vector *v)
{
    return v->capacity;
}

/**
 * @brief vector_empty - является ли динамический массив пустым
 * @param v - указатель на структуру динамического массива
 * @return если динамический массив пуст, возвращается 1, в противном случае 0
 */
int vector_empty(struct vector *v)
{
    if (v->size > 0)
        return 0;
    else
        return 1;
}

/**
 * @brief vector_data
 * @param v - указатель на структуру динамического массива
 * @return указатель на начало массива указателей на элементы, если динамический массив не пуст
 */
void **vector_data(struct vector *v)
{
    if (vector_empty(v))
        return NULL;
    else
        return v->data;
}

/**
 * @brief vector_push_back - выделяет памяти под элемент в конце внутреннего массива указателей и копирует данные из data
 * @param v - указатель на структуру динамического массива
 * @param data - указатель на пользовательские данные
 */
void vector_push_back(struct vector *v, USED_DATA_TYPE *data)
{
    /* Если количество элементов массива достигло объёма, то необходимо выделить ещё больше памяти */
    if (v->size == v->capacity)
    {
        if (v->capacity >= 4096)
            v->capacity = v->capacity * 3 / 2;
        else
            v->capacity *= 2;

        vector_realloc(&v);
    }

    v->data[v->size] = malloc(v->data_sizeof);
    memcpy(v->data[v->size], data, v->data_sizeof);
    v->size += 1;
}

/**
 * @brief vector_erase - удаляет из динамического массива элемент с индексом index
 * @param v - указатель на структуру динамического массива
 * @param index - индекс, в котором находится искомый элемент
 */
void vector_erase(struct vector *v, size_t index)
{
    if (index >= v->size || vector_empty(v))
        return;

    free(v->data[index]); /* Очистка памяти, занимаемой элементов */
    memmove(v->data + index, v->data + index + 1, (v->size - index) * sizeof(void*)); /* Сдвиг указателей внутреннего массива на 1 элемент влева */

    v->size -= 1;
    if (v->size > 4 && v->size < v->capacity / 2) /* Перевыделение памяти при уменьшении количества элементов */
    {
        v->capacity /= 2;
        vector_realloc(&v);
    }
}

/**
 * @brief vector_pop_back - то же, что и vector_erase(v, 0)
 * @param v - указатель на структуру динамического массива
 */
void vector_pop_back(struct vector *v)
{
    if (vector_empty(v))
        return;

    vector_erase(v, v->size - 1);
}

/**
 * @brief vector_insert - вставляет элемент data на место, указываемое в index
 * @param v - указатель на структуру динамического массива
 * @param data - указатель на пользовательские данные
 * @param index - индекс, в который необходимо поместить элемент
 */
void vector_insert(struct vector *v, USED_DATA_TYPE *data, size_t index)
{
    if (index >= v->size)
        return;

    v->size += 1;
    if (v->size == v->capacity)
    {
        if (v->capacity >= 4096)
            v->capacity = v->capacity * 3 / 2;
        else
            v->capacity *= 2;

        vector_realloc(&v);
    }

    memmove(v->data + index + 1, v->data + index, (v->size - index - 1) * sizeof(void*));
    v->data[index] = malloc(v->data_sizeof);
    memcpy(v->data[index], data, v->data_sizeof);
}

/**
 * @brief vector_at возвращает пользовательский элемент, находящийся ну позиции index в динамическом массиве
 * @param v - указатель на структуру динамического массива
 * @param index
 * @return
 */
void * vector_at(struct vector *v, size_t index)
{
    if (index >= v->size)
        return NULL;

    return v->data[index];
}

/**
 * @brief vector_swap меняет местами два пользовательских элемента на позициях first и second
 * @param v - указатель на структуру динамического массива
 * @param first - индекс первого элемента
 * @param second - индекс второго элемента
 */
void vector_swap(struct vector *v, size_t first, size_t second)
{
    if (first >= v->size || second >= v->size)
        return;

    void *tmp = v->data[first];
    v->data[first] = v->data[second];
    v->data[second] = tmp;
}

/**
 * @brief vector_move - перемещает элемент с индекса pos на индекс new_pos
 * @param v - указатель на структуру динамического массива
 * @param pos - первоначальная позиция элемента
 * @param new_pos - конечныая позиция элемента
 */
void vector_move(struct vector *v, size_t pos, size_t new_pos)
{
    if (pos >= v->size || new_pos >= v->size)
        return;

    void *tmp = malloc(v->data_sizeof);
    memcpy(tmp, vector_at(v, pos), v->data_sizeof);

    vector_erase(v, pos);
    vector_insert(v, tmp, new_pos);

    free(tmp);
}

/**
 * @brief vector_clear - очищает динамический массив, освобождая память
 * @param v - указатель на структуру динамического массива
 */
void vector_clear(struct vector *v)
{
    for (size_t i = 0; i < v->size; ++i)
    {
        free(v->data[i]);
    }

    free(v->data);

    vector_init_memory(&v);
}

/**
 * @brief vector_destroy - очищает динамический массив и очищает память, занимаемую самим динамическим массивом
 * @param v - указатель на указатель структуры динамического массива
 */
void vector_destroy(struct vector **v)
{
    vector_clear(*v);
    free(*v);
}

// Приватные функции
void vector_realloc(struct vector **v)
{   
    void **tmp = (void**)realloc((*v)->data, (*v)->capacity * sizeof(void*));
    (*v)->data = tmp;
}

void vector_init_memory(struct vector **v)
{
    (*v)->size = 0;
    (*v)->capacity = 4;
    (*v)->data = (void**)malloc((*v)->capacity * sizeof(void*));

    if (!(*v)->data)
    {
        /* Если невозможно выделить память, значит дальше делать нечего - в системе катастрофические проблемы */
        exit(1);
    }
}

#endif // VECTOR_H
