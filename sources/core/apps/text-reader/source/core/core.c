#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>

#include <linux/fb.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <cjson/cJSON.h> 

#include <kot-graphics/font.h>
#include <kot-graphics/utils.h>
#include <kot-graphics/image.h>

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define FONT_WIDTH 8
#define FONT_HEIGHT 16

#define TAB_CHAR_COUNT          (4)
#define HEADER_BACKGROUND       (0x111111)
#define IMAGE_BORDER            (0xc70039)
#define WIDTH_MARGIN            (50)
#define MARGIN_HEIGHT           (50)
#define HEADER_SIZE             (14)
#define HEADER_HEIGHT           (26)
#define TEXT_COLOR              (0xffffff)
#define INFO_SIZE               (16)
#define WRITE_AREA_MARGIN       (5)
#define LINE_COUNT_MAX_CHAR     (4)
#define LINE_COUNT_WIDTH        (FONT_WIDTH * LINE_COUNT_MAX_CHAR + WRITE_AREA_MARGIN * 3)

kfont_t font;

int fb_fd = -1;
kframebuffer_t fb;
struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;

kframebuffer_t* text_framebuffer = NULL;

char* font_path = NULL;
char* wallpaper_path = NULL;
char* font_bitmap_path = NULL;
color_t background_color = 0;
color_t line_count_color = 0;

size_t text_file_size = 0;
size_t text_index_start = 0;
char* text_buffer = NULL;
FILE* text_file = NULL;

size_t copy_offset = 0;

uint8_t* font_bitmap_data = NULL;
size_t font_bitmap_file_size = 0;

size_t json_size = 0;
void* json_buffer = NULL;
FILE* json_file = NULL;
cJSON* json_root = NULL;
cJSON* font_path_json = NULL;
cJSON* font_bitmap_path_json = NULL;
cJSON* wallpaper_path_json = NULL;
cJSON* background_color_json = NULL;
cJSON* line_count_color_json = NULL;

uint32_t char_row_count = 0;
uint32_t char_col_count = 0;

uint32_t write_area_x = 0;
uint32_t write_area_y = 0;
uint32_t write_area_width = 0;
uint32_t write_area_height = 0;

void bitmap_setchar(kframebuffer_t* framebuffer, uint32_t cx_ppos, uint32_t cy_ppos, char c, color_t fg_color){
    uint8_t* glyph = &font_bitmap_data[c*((FONT_WIDTH*FONT_HEIGHT)/8)];
    
    for(uint32_t y = 0; y < FONT_HEIGHT; y++) {
        for(uint32_t x = 0; x < FONT_WIDTH; x++) {
            
            if(glyph[y] & (1 << FONT_WIDTH - x)) {
                put_pixel(framebuffer, cx_ppos + x, cy_ppos, fg_color); 
            }
        }
        cy_ppos++;
    }
}

uint64_t get_ticks_ms(){
    struct timeval tv; 
    gettimeofday(&tv, NULL);
    uint64_t milliseconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return milliseconds;
}

void draw_frame(){
    write(fb_fd, fb.buffer, fb.size);
}

void scroll_down(int i){
    for(int a = 0; a < i; a++){
        if(copy_offset > text_framebuffer->pitch * FONT_HEIGHT){
            copy_offset -= text_framebuffer->pitch * FONT_HEIGHT;
        }else{
            copy_offset = 0;
        }
    }
}


void scroll_up(int i){
    for(int a = 0; a < i; a++){
        if((text_framebuffer->size - copy_offset) > (write_area_height * text_framebuffer->pitch)){
            copy_offset += text_framebuffer->pitch * FONT_HEIGHT;
        }else if(text_framebuffer->size > (write_area_height * text_framebuffer->pitch)){
            copy_offset = text_framebuffer->size - (write_area_height * text_framebuffer->pitch);
        }
    }
}

int get_key(int* pressed, uint64_t* key){
    uint64_t buffer[2];
    if(read(fb_fd, &buffer, sizeof(uint64_t) * 2) > 0){
        if(buffer[1] & ((uint64_t)1 << 63)){
            *pressed = true;
        }else{
            *pressed = false;
        }
        *key = buffer[0];
        return 1;
    }
    return 0;
}

int get_input(){
    static bool arrow_pressed = false;
    static bool wait_release = false;
    static bool wait_release_up = false;
    static bool wait_release_down = false;
    static bool wait_release_right = false;
    static bool wait_release_left = false;
    
    int pressed = 0;
    uint64_t key = 0;
    int ret = get_key(&pressed, &key);

    if(ret){
        if(key == 96 && !arrow_pressed){
            arrow_pressed = true;
        }else{
            if(arrow_pressed && pressed){
                if(key == 72 && !wait_release_up){
                    // up
                    wait_release_up = true;
                    scroll_down(1);
                }else if(key == 80 && !wait_release_down){
                    // down
                    wait_release_down = true;
                    scroll_up(1);
                }else if(key == 77 && !wait_release_right){
                    // right
                    wait_release_right = true;
                    scroll_up(char_col_count);
                }else if(key == 75 && !wait_release_left){
                    // left
                    wait_release_left = true;
                    scroll_down(char_col_count);
                }

                arrow_pressed = false;
            }else if(arrow_pressed && !pressed){
                if(key == 72 && wait_release_up){
                    // up
                    wait_release_up = false;
                }else if(key == 80 && wait_release_down){
                    // down
                    wait_release_down = false;
                }else if(key == 77 && wait_release_right){
                    // down
                    wait_release_right = false;
                }else if(key == 75 && wait_release_left){
                    // down
                    wait_release_left = false;
                }
                arrow_pressed = false;
            }else if(key == 1 && pressed){
                // exit
                return 1;
            }
        }
    }
    return 0;
}


char* get_file_ext(char* name) {
    char* dot = strrchr(name, '.');
    if(!dot || dot == name) return "";
    return dot + 1;
}

int load_fb(){
    fb_fd = open("/dev/fb0", O_RDWR);

    assert(fb_fd >= 0);


    assert(ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix_screeninfo) == 0);
    assert(ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_screeninfo) == 0);

    if(
        fix_screeninfo.visual == FB_VISUAL_TRUECOLOR && 
        var_screeninfo.bits_per_pixel == 32 &&
        var_screeninfo.red.length == 8 && var_screeninfo.red.msb_right == 0 &&
        var_screeninfo.green.length == 8 && var_screeninfo.green.msb_right == 0 &&
        var_screeninfo.blue.length == 8 && var_screeninfo.blue.msb_right == 0 
    ){
        fb.bpp = var_screeninfo.bits_per_pixel;
        fb.btpp = fb.bpp / 8;
        fb.size = var_screeninfo.xres_virtual * var_screeninfo.yres_virtual * fb.btpp;
        fb.width = var_screeninfo.xres_virtual;
        fb.pitch = fb.width * fb.btpp;
        fb.height = var_screeninfo.yres_virtual;
        fb.buffer = malloc(fb.size);
        memset(fb.buffer, 0, fb.size);
    }else{
        perror("'/dev/fb0' : format not supported");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int load_json(){
    json_file = fopen("/usr/bin/res/text-reader/text-reader.json", "r"); 

    if(json_file == NULL){ 
        perror("Error: Unable to open the file : /usr/bin/res/text-reader/text-reader.json\n"); 
        return EXIT_FAILURE; 
    } 
  
    fseek(json_file, 0, SEEK_END);
    json_size = ftell(json_file);
    fseek(json_file, 0, SEEK_SET);
    
    json_buffer = malloc(json_size);
    fread(json_buffer, 1, json_size, json_file); 
  
    json_root = cJSON_Parse(json_buffer); 

    if(json_root != NULL){
        font_path_json = cJSON_GetObjectItem(json_root, "font_path");
        if(cJSON_IsString(font_path_json) && (font_path_json->valuestring != NULL)){
            font_path = strdup(font_path_json->valuestring);
        }else{
            free(json_buffer);

            fclose(json_file);
            
            printf("Error: Unable to parse the file : /usr/bin/res/text-reader/text-reader.json\n"); 
            return EXIT_FAILURE; 
        }

        font_bitmap_path_json = cJSON_GetObjectItem(json_root, "font_bitmap_path");
        if(cJSON_IsString(font_bitmap_path_json) && (font_bitmap_path_json->valuestring != NULL)){
            font_bitmap_path = strdup(font_bitmap_path_json->valuestring);
        }else{
            free(json_buffer);

            fclose(json_file);
            
            printf("Error: Unable to parse the file : /usr/bin/res/text-reader/text-reader.json\n"); 
            return EXIT_FAILURE; 
        }

        wallpaper_path_json = cJSON_GetObjectItem(json_root, "wallpaper_path");
        if(cJSON_IsString(wallpaper_path_json) && (wallpaper_path_json->valuestring != NULL)){
            wallpaper_path = strdup(wallpaper_path_json->valuestring);
        }else{
            free(json_buffer);

            fclose(json_file);
            
            printf("Error: Unable to parse the file : /usr/bin/res/text-reader/text-reader.json\n"); 
            return EXIT_FAILURE; 
        }

        background_color_json = cJSON_GetObjectItem(json_root, "background_color");
        if(cJSON_IsNumber(background_color_json)){
            background_color = background_color_json->valueint;
        }else{
            free(json_buffer);

            fclose(json_file);
            
            printf("Error: Unable to parse the file : /usr/bin/res/text-reader/text-reader.json\n"); 
            return EXIT_FAILURE; 
        }

        line_count_color_json = cJSON_GetObjectItem(json_root, "line_count_color");
        if(cJSON_IsNumber(line_count_color_json)){
            line_count_color = line_count_color_json->valueint;
        }else{
            free(json_buffer);

            fclose(json_file);
            
            printf("Error: Unable to parse the file : /usr/bin/res/text-reader/text-reader.json\n"); 
            return EXIT_FAILURE; 
        }
    }else{
        free(json_buffer);

        fclose(json_file);

        printf("Error: Unable to parse the file : /usr/bin/res/text-reader/text-reader.json\n"); 
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}

int load_font_data(){
    FILE* font_bitmap_file = fopen(font_bitmap_path, "rb");
    if(font_bitmap_file == NULL){
        perror("Failed to open binary font file");
        fclose(json_file);
        close(fb_fd);
        return EXIT_FAILURE;
    }

    fseek(font_bitmap_file, 0, SEEK_END);
    font_bitmap_file_size = ftell(font_bitmap_file);
    fseek(font_bitmap_file, 0, SEEK_SET);

    font_bitmap_data = malloc(font_bitmap_file_size);
    fread(font_bitmap_data, font_bitmap_file_size, 1, font_bitmap_file);
    fclose(font_bitmap_file);

    FILE* font_file = fopen(font_path, "rb");
    if(font_file == NULL){
        perror("Failed to open font file");
        fclose(json_file);
        close(fb_fd);
        return EXIT_FAILURE;
    }

    fseek(font_file, 0, SEEK_END);
    size_t font_file_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    void* font_data = malloc(font_file_size);
    fread(font_data, font_file_size, 1, font_file);
    fclose(font_file);

    font = load_font(font_data, font_file_size);
    free(font_data);

    if(font == NULL){
        return EXIT_FAILURE;
    }else{
        return EXIT_SUCCESS;
    }
}

int load_text_file(char* path){
    FILE* text_file = fopen(path, "rb");

    fseek(text_file, 0, SEEK_END);
    text_file_size = ftell(text_file);
    fseek(text_file, 0, SEEK_SET);

    text_buffer = malloc(text_file_size);
    fread(text_buffer, text_file_size, 1, text_file);
    fclose(text_file);

    return EXIT_SUCCESS;
}

uint32_t get_buffer_height(){
    char* c = &text_buffer[text_index_start];
    uint32_t cx_pos = 0;
    uint32_t cy_pos = FONT_HEIGHT;

    uint32_t col_count = 0;
    uint32_t row_count = 0;
    uint32_t c_count = 0;
    while(*c){
        if(*c == '\n'){
            row_count = 0;
            cx_pos = 0;
            cy_pos += FONT_HEIGHT;
            col_count++;
        }else if(*c == '\t'){
            row_count++;
            cx_pos += FONT_WIDTH;

            row_count -= row_count % TAB_CHAR_COUNT;
            row_count += TAB_CHAR_COUNT;
            cx_pos -= cx_pos % (TAB_CHAR_COUNT * FONT_WIDTH);
            cx_pos += TAB_CHAR_COUNT * FONT_WIDTH;
        }else{
            if(row_count >= char_row_count){
                row_count = 0;
                cx_pos = 0;
                cy_pos += FONT_HEIGHT;
                col_count++;
                if(col_count >= char_col_count){
                    break;
                }
            }

            cx_pos += FONT_WIDTH;
            row_count++;
        }

        c_count++;
        c++;
    }

    return cy_pos;
}

kframebuffer_t* load_buffer(){
    kframebuffer_t* framebuffer = malloc(sizeof(kframebuffer_t));
    framebuffer->bpp = fb.bpp;
    framebuffer->btpp = fb.btpp;
    framebuffer->width = write_area_width;
    framebuffer->height = get_buffer_height();
    framebuffer->pitch = framebuffer->width * framebuffer->btpp;
    framebuffer->size = framebuffer->pitch * framebuffer->height;
    framebuffer->buffer = malloc(framebuffer->size);

    draw_rectangle(framebuffer, 0, 0, framebuffer->width, framebuffer->height, background_color);
    draw_rectangle(framebuffer, LINE_COUNT_WIDTH - (WRITE_AREA_MARGIN / 2) - 1, 0, 1, framebuffer->height, line_count_color);

    char* c = &text_buffer[text_index_start];
    uint32_t cx_pos = LINE_COUNT_WIDTH;
    uint32_t cy_pos = 0;

    uint32_t col_count = 0;
    uint32_t row_count = 0;
    uint32_t c_count = 0;

    {
        char* line_count_str = NULL;
        if(col_count + 1 < 10000){
            assert(asprintf(&line_count_str, "%04d", col_count + 1) >= 0);
        }else{
            assert(asprintf(&line_count_str, "++++") >= 0);
        }
        uint32_t line_count_x = 0;
        char* line_count_char_inc = line_count_str;
        while(*line_count_char_inc){
            line_count_x += FONT_WIDTH;
            bitmap_setchar(framebuffer, line_count_x, cy_pos, *line_count_char_inc, TEXT_COLOR);
            line_count_char_inc++;
        }
        free(line_count_str);
    }

    while(*c){
        if(*c == '\n'){
            row_count = 0;
            cx_pos = LINE_COUNT_WIDTH;
            cy_pos += FONT_HEIGHT;
            col_count++;
            char* line_count_str = NULL;
            if(col_count + 1 < 10000){
                assert(asprintf(&line_count_str, "%04d", col_count + 1) >= 0);
            }else{
                assert(asprintf(&line_count_str, "++++") >= 0);
            }
            uint32_t line_count_x = 0;
            while(*line_count_str){
                line_count_x += FONT_WIDTH;
                bitmap_setchar(framebuffer, line_count_x, cy_pos, *line_count_str, TEXT_COLOR);
                line_count_str++;
            }
        }else if(*c == '\t'){
            row_count++;
            cx_pos += FONT_WIDTH;

            row_count -= row_count % TAB_CHAR_COUNT;
            row_count += TAB_CHAR_COUNT;
            cx_pos -= (cx_pos - LINE_COUNT_WIDTH) % (TAB_CHAR_COUNT * FONT_WIDTH);
            cx_pos += TAB_CHAR_COUNT * FONT_WIDTH;
        }else{
            if(row_count >= char_row_count){
                row_count = 0;
                cx_pos = LINE_COUNT_WIDTH;
                cy_pos += FONT_HEIGHT;
                col_count++;
                if(col_count >= char_col_count){
                    break;
                }
            }

            bitmap_setchar(framebuffer, cx_pos, cy_pos, *c, TEXT_COLOR);
            cx_pos += FONT_WIDTH;
            row_count++;
        }

        c_count++;
        c++;
    }

    return framebuffer;
}

int main(int argc, char* argv[]){
    if(argc == 2){
        if(load_fb() != EXIT_SUCCESS){
            return EXIT_FAILURE;
        }

        if(load_json() != EXIT_SUCCESS){
            return EXIT_FAILURE;
        }

        if(load_font_data() != EXIT_SUCCESS){
            cJSON_Delete(json_root);

            free(json_buffer);

            fclose(json_file);

            return EXIT_FAILURE;
        }

        if(load_text_file(argv[1]) != EXIT_SUCCESS){
            free(font_path);
            free(wallpaper_path);
            free(font_bitmap_path);

            cJSON_Delete(json_root);

            free(json_buffer);

            fclose(json_file);

            return EXIT_FAILURE;
        }


        char* extension = get_file_ext(argv[1]);

        raw_image_t* wallpaper = load_jpeg_image_file(wallpaper_path);
        raw_image_t* wallpaper_resized = NULL;
        if(wallpaper != NULL){
            if(wallpaper->width < wallpaper->height){
                wallpaper_resized = resize_image(wallpaper, 0, fb.height, true);
            }else{
                wallpaper_resized = resize_image(wallpaper, fb.width, 0, true);
            }
            draw_image(&fb, wallpaper_resized, 0, HEADER_HEIGHT, fb.width, fb.height - HEADER_HEIGHT);
        }
        load_pen(font, &fb, 0, 0, INFO_SIZE, 0, TEXT_COLOR);
        kfont_pos_t x = get_pen_pos_x(font);
        kfont_pos_t y = fb.height - INFO_SIZE - 10;
        set_pen_color(font, ~TEXT_COLOR);
        write_paragraph(font, -1, fb.height - INFO_SIZE - 10, fb.width, PARAGRAPH_CENTER, "Exit : <Esc>\n", -1);
        set_pen_color(font, TEXT_COLOR);
        write_paragraph(font, x - 1, y - 1, fb.width, PARAGRAPH_CENTER, "Exit : <Esc>\n", -1);

        set_pen_size(font, HEADER_SIZE);

        write_area_x = WIDTH_MARGIN;
        write_area_y = HEADER_HEIGHT + MARGIN_HEIGHT;
        write_area_width = fb.width - (WIDTH_MARGIN * 2) - (WRITE_AREA_MARGIN * 2);
        write_area_height = fb.height - (MARGIN_HEIGHT * 2) - HEADER_HEIGHT - (WRITE_AREA_MARGIN * 2);

        if(write_area_width % FONT_WIDTH){
            write_area_x += (write_area_width % FONT_WIDTH) / 2;
            write_area_width -= write_area_width % FONT_WIDTH;
        }

        if(write_area_height % FONT_HEIGHT){
            write_area_y += (write_area_height % FONT_HEIGHT) / 2;
            write_area_height -= write_area_height % FONT_HEIGHT;
        }

        char_row_count = write_area_width / FONT_WIDTH;
        char_col_count = write_area_height / FONT_HEIGHT;

        draw_rectangle(&fb, write_area_x - WRITE_AREA_MARGIN, write_area_y - WRITE_AREA_MARGIN, write_area_width + (2 * WRITE_AREA_MARGIN), write_area_height + (2 * WRITE_AREA_MARGIN), background_color);
        
        text_framebuffer = load_buffer();
        
        while(!get_input()){
            draw_rectangle(&fb, 0, 0, fb.width, HEADER_HEIGHT, HEADER_BACKGROUND);
            blit_framebuffer(&fb, text_framebuffer, write_area_x, write_area_y, write_area_height, copy_offset);

            char* image_reader_info = NULL;
            int percentage = 100;
            if(text_framebuffer->height > write_area_height){
                percentage = (((copy_offset / text_framebuffer->pitch) + write_area_height) * 100) / text_framebuffer->height;
            }
            assert(asprintf(&image_reader_info, "%s | %d%%", argv[1], percentage) >= 0);
            write_paragraph(font, 0, 0, fb.width, PARAGRAPH_CENTER, image_reader_info, -1);

            draw_frame();
        }


        free(font_path);
        free(wallpaper_path);
        free(font_bitmap_path);

        cJSON_Delete(json_root);

        free(json_buffer);

        free(text_buffer);

        fclose(json_file);

        return EXIT_SUCCESS;
    }else{
        printf("\033[1;31musage: text-reader file_path\033[0m"); 
        printf("Press <Enter> to Continue\n");
        getchar();
        return EXIT_FAILURE;
    }
}