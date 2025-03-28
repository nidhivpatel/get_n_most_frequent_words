#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_WORD_LENGTH 100 //Assume maximum word length is 100
#define HASH_TABLE_SIZE 10009 //Assume number of unique words should not be more than hash table size
#define WORD_SEPARATER_CHARACTERS " \t\n,.?!:;"
#define MAX_TEXT_SIZE 1000000

struct Frequency {
    char word[MAX_WORD_LENGTH];
    int32_t count;
    struct Frequency *next;
};

struct HashTable {
    struct Frequency *table[HASH_TABLE_SIZE];
};

struct Heap {
    int32_t size;
    int32_t capacity;
    struct Frequency *data;
};

// DJB2 Hashing algorithm
unsigned int get_hash_value(const char *str) {
    unsigned int hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + (unsigned char)(*str++);
    }
    return hash % HASH_TABLE_SIZE;
}

void insert_in_hash_table(struct HashTable *ht, const char *word) {
    unsigned int index = get_hash_value(word);
    struct Frequency *current = ht->table[index];
    while (current) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            return;
        }
        current = current->next;
    }
    struct Frequency *new_word = malloc(sizeof(struct Frequency));
    strcpy(new_word->word, word);
    new_word->count = 1;
    new_word->next = ht->table[index];
    ht->table[index] = new_word;
}

void clear(struct HashTable ht) {
    for (int32_t i = 0; i < HASH_TABLE_SIZE; ++i) {
        struct Frequency *current = ht.table[i];
        struct Frequency *prev;
        while (current) {
            prev = current;
            current = current->next;
            free(prev);
        }
    }
}

void swap(struct Frequency *a, struct Frequency *b) {
    struct Frequency temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(struct Heap *heap, int32_t index) {
    int32_t parent = (index - 1) / 2;
    while (index > 0 && heap->data[index].count < heap->data[parent].count) {
        swap(&heap->data[index], &heap->data[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapify_down(struct Heap *heap, int32_t index) {
    int32_t smallest = index;
    int32_t left = 2 * index + 1;
    int32_t right = 2 * index + 2;
    if (left < heap->size && heap->data[left].count < heap->data[smallest].count)
        smallest = left;
    if (right < heap->size && heap->data[right].count < heap->data[smallest].count)
        smallest = right;
    if (smallest != index) {
        swap(&heap->data[index], &heap->data[smallest]);
        heapify_down(heap, smallest);
    }
}

void insert_in_heap(struct Heap *heap, struct Frequency word) {
    if (heap->size < heap->capacity) {
        heap->data[heap->size] = word;
        heapify_up(heap, heap->size);
        heap->size++;
    } else if (word.count > heap->data[0].count) {
        heap->data[0] = word;
        heapify_down(heap, 0);
    }
}

void to_lowercase(char *str) {
    for (int32_t i = 0; str[i]; ++i) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

size_t read_file(const char *path, char *text, int max_length) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("Could not open the file!\n");
        return 1;
    }
    
    size_t len = fread(text, sizeof(char), max_length - 1, file);
    fclose(file);
    text[len] = '\0';
    return len;
    
    //Sample Test Case
    //strcpy(text, "Nidhi, is Nidhi, is: crazy is's is crazy: crazy, crazy");
    //return strlen(text);
}

char **find_frequent_words(const char *path, int32_t n) {
    char text[MAX_TEXT_SIZE];
    size_t len = read_file(path, text, MAX_TEXT_SIZE);

    struct HashTable ht = {0};
    struct Heap heap;
    heap.size = 0;
    heap.capacity = n;
    heap.data = malloc(n * sizeof(struct Frequency));
    
    char *word = strtok(text, WORD_SEPARATER_CHARACTERS);
    int32_t word_count = 0;
    
    while (word) {
        to_lowercase(word);
        insert_in_hash_table(&ht, word);
        word = strtok(NULL, WORD_SEPARATER_CHARACTERS);
    }
    
    for (int32_t i = 0; i < HASH_TABLE_SIZE; ++i) {
        struct Frequency *ht_word = ht.table[i];
        while (ht_word) {
            insert_in_heap(&heap, *ht_word);
            ht_word = ht_word->next;
        }
    }
    
    char **result = malloc(n * sizeof(char *));
    for (int32_t i = 0; i < n && i < heap.size; ++i) {
        result[i] = strdup(heap.data[i].word);
    }
    
    free(heap.data);
    clear(ht);
    return result;
}

int main() {
    char *path = "shakespeare.txt"; //write path to TensorFlow Shakespeare dataset file.
    int32_t n = 10;
    char **frequent_words = find_frequent_words(path, n);
    
    printf("%d most frequent words:\n", n);
    for (int32_t i = 0; i < n; ++i) {
        printf("%s\n", frequent_words[i]);
        free(frequent_words[i]);
    }
    free(frequent_words);
    
    return 0;
}

//Time Complexity: O(t + wlogn)
//where t = total length of text
// w = no of distinct word
// n = input