
int minimap_win;

void display_minimap();
int display_minimap_handler(window_info *win);

//called when map changes
void change_minimap();

//called when player moves
void update_exploration_map();
