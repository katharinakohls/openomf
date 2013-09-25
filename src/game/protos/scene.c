#include "game/protos/scene.h"
#include "video/video.h"
#include "resources/ids.h"
#include "resources/bk_loader.h"
#include "utils/log.h"
#include "game/game_state.h"

// Loads BK file etc.
int scene_create(scene *scene, void *game_state, unsigned int scene_id) {
    // Load BK
    if(scene_id == SCENE_NONE || load_bk_file(&scene->bk_data, scene_id)) {
        PERROR("Unable to load BK file!");
        return 1;
    }

    scene->game_state = game_state;
    scene->userdata = NULL;
    scene->free = NULL;
    scene->event = NULL;
    scene->render = NULL;
    scene->tick = NULL;
    return 0;
}

void scene_set_userdata(scene *scene, void *userdata) {
    scene->userdata = userdata;
}

void* scene_get_userdata(scene *scene) {
    return scene->userdata;
}

// Return 0 if event was handled here
int scene_event(scene *scene, SDL_Event *event) {
    if(scene->event != NULL) {
        return scene->event(scene, event);
    }
    return 1;
}

void scene_render(scene *scene) {
    video_render_background(&scene->bk_data.background.tex);
    if(scene->render != NULL) {
        scene->render(scene);
    }
}

void scene_tick(scene *scene) {
    if(scene->tick != NULL) {
        scene->tick(scene);
    }
}

void scene_free(scene *scene) {
    if(scene->free != NULL) {
        scene->free(scene);
    }
    bk_free(&scene->bk_data);
}

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc) {
    scene->free = cbfunc;
}

void scene_set_event_cb(scene *scene, scene_event_cb cbfunc) {
    scene->event = cbfunc;
}

void scene_set_render_cb(scene *scene, scene_render_cb cbfunc) {
    scene->render = cbfunc;
}

void scene_set_tick_cb(scene *scene, scene_tick_cb cbfunc) {
    scene->tick = cbfunc;
}

void scene_load_new_scene(scene *scene, unsigned int scene_id) {
    game_state_set_next(scene->game_state, scene_id);
}

void scene_add_object(scene *scene, object *obj) {
    game_state_add_object(scene->game_state, obj);
}

int scene_is_valid(int id) {
    switch(id) {
        case SCENE_INTRO:
        case SCENE_MENU:
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
        case SCENE_ARENA5:
        case SCENE_NEWSROOM:
        case SCENE_END:
        case SCENE_END1:
        case SCENE_END2: 
        case SCENE_CREDITS:
        case SCENE_MECHLAB:
        case SCENE_MELEE:
        case SCENE_VS:
        case SCENE_NORTHAM:
        case SCENE_KATUSHAI:
        case SCENE_WAR:
        case SCENE_WORLD:
            return 1;
    }
    return 0;
}



