#pragma once

#include <kot/types.h>

#include <kot-graphics++/utils.h>

#include <kot/utils/vector.h>

using namespace Graphic;

namespace Ui {

    enum ComponentPosition {
        RELATIVE, // default
        ABSOLUTE,
    };

    class Component {

        public:
            typedef struct {
                ComponentPosition position;

                uint32_t width;
                uint32_t height;
                uint16_t fontSize;
                uint16_t borderRadius;

                uint32_t backgroundColor;
                uint32_t foregroundColor;

                uint32_t x;
                uint32_t y;
            } ComponentStyle;

            Component(ComponentStyle style);
            Component(framebuffer_t* fb, ComponentStyle style);

            void draw();
            
            void edit();
            // todo: remove, edit

            void addChild(Component* child);
            
        private:
            framebuffer_t* fb;
            ComponentStyle* style;
            Component* parent;
            vector_t* childs;
            uint16_t type;

    };

    /* components */

    typedef struct {
        uint32_t width;
        uint32_t height;
        uint32_t color;
    } BoxStyle;
    Component* box(BoxStyle style);

    typedef struct {
        uint32_t backgroundColor;
        uint32_t foregroundColor;
    } TitlebarStyle;
    Component* titlebar(char* title, TitlebarStyle style);

}