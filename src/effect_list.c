#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "effect_list.h"

#include <esp_log.h>
#include <stdlib.h>

#include "effect_functions.h"

typedef struct list_node {
    effect_t *effect;
    struct list_node *next;
} list_node_t;

struct effect_list {
    list_node_t *head;
};

effect_list_t effect_list = NULL;

effect_list_t effect_list_new() {
    effect_list_t list = malloc(sizeof *list);
    if (list) {
        list->head = NULL;
        ESP_LOGD(__FILE__, "effect_list created");
    }
    return list;
}

effect_list_t effect_list_add(effect_list_t list, effect_t *effect) {
    if (!list) {
        return list;
    }
    list_node_t *node = malloc(sizeof *node);
    node->effect = effect;
    node->next = NULL;

    list_node_t *head = list->head;
    if (NULL == head) {
        list->head = node;
        return list;
    }

    list_node_t *current = head;
    while (NULL != current->next) {
        current = current->next;
    }
    current->next = node;

    return list;
}

effect_t *effect_list_first(effect_list_t list) {
    return list->head->effect;
}

int effect_list_done(effect_list_t list, effect_t *e) {
    list_node_t *head = list->head;
    if (NULL == head) {
        return 1;
    }

    list_node_t *current = head;
    while (current != NULL) {
        if (e == current->effect) {
            return NULL == current->next;
        }
        current = current->next;
    }

    return -1;
}

effect_t *effect_list_next(effect_list_t list, effect_t *e) {
    list_node_t *head = list->head;
    if (NULL == head) {
        return NULL;
    }

    list_node_t *current = head;
    while (current != NULL) {
        if (e == current->effect) {
            if (NULL == current->next) {
                return NULL;
            }
            return current->next->effect;
        }
        current = current->next;
    }

    return NULL;
}

int effect_list_length(effect_list_t list) {
    int len = 0;
    list_node_t *head = list->head;
    if (NULL == head) {
        return 0;
    }

    list_node_t *current = head;
    while (current != NULL) {
        len += 1;
        current = current->next;
    }
    return len;
}

effect_t *effect_list_get_by_idx(effect_list_t list, int idx) {
    list_node_t *head = list->head;
    if (NULL == head) {
        return NULL;
    }

    if (idx >= effect_list_length(list)) {
        return NULL;
    }

    list_node_t *current = list->head;
    for (int i = 0; i < idx; i++) {
        current = current->next;
    }

    return current->effect;
}

void init_effect_list() {
    effect_list = effect_list_new();
    effect_list = effect_list_add(effect_list, effect_new("fireworks", fireworks, NULL));
    effect_list = effect_list_add(effect_list, effect_new("ball", ball, NULL));
    effect_list = effect_list_add(effect_list, effect_new("linespin", linespin, NULL));
    effect_list = effect_list_add(effect_list, effect_new("shift_planes", shift_planes, NULL));
    effect_list = effect_list_add(effect_list, effect_new("ripples", ripples, NULL));
    effect_list = effect_list_add(effect_list, effect_new("sinelines", sinelines, NULL));
    effect_list = effect_list_add(effect_list, effect_new("shift_suspend", shift_suspend, NULL));
    effect_list = effect_list_add(effect_list, effect_new("sidewaves", sidewaves, NULL));
    effect_list = effect_list_add(effect_list, effect_new("mic_spectrum", mic_spectrum, mic_spectrum_stop));
    effect_list = effect_list_add(effect_list, effect_new("cube_color_wheel", cube_color_wheel, NULL));
    effect_list = effect_list_add(effect_list, effect_new("vertical_travelers", vertical_travelers, NULL));
    effect_list = effect_list_add(effect_list, effect_new("wireframes", wireframes, NULL));
    effect_list = effect_list_add(effect_list, effect_new("rain", rain, NULL));
    effect_list = effect_list_add(effect_list, effect_new("wave_color_wheel", wave_color_wheel, NULL));

    ESP_LOGI(__FILE__, "Total number of effects: %d", effect_list_length(effect_list));
}
