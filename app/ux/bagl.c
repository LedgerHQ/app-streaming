
/*******************************************************************************
*   Ledger Nano S - Secure firmware
*   (c) 2021 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "config.h"
#include "bagl.h"
#include "sdk.h"

#include <string.h>

//#include <stdio.h>

//#include "os.h"

// --------------------------------------------------------------------------------------
// API
// -------------------------------------------------------------------------------------- 

// --------------------------------------------------------------------------------------
void bagl_draw_bg(unsigned int color) {
  bagl_component_t c;
  memset(&c, 0, sizeof(c));
  c.type = BAGL_RECTANGLE;
  c.userid = BAGL_NONE;
  c.fgcolor = color;
  c.x = 0;
  c.y = 0;
  c.width = BAGL_WIDTH;
  c.height = BAGL_HEIGHT;
  c.fill = BAGL_FILL;
  // draw the rect
  bagl_draw_with_context(&c, NULL, 0, 0);
}

// --------------------------------------------------------------------------------------
void bagl_draw_glyph(const bagl_component_t* component, const bagl_icon_details_t* icon_details) {
  // no space to display that
  if (icon_details->bpp > 2) {
    return;
  }

  /*
  // take into account the remaining bits not byte aligned
  unsigned int w = ((component->width*component->height*icon_details->bpp)/8);
  if (w%8) {
    w++;
  }
  */
 
  // draw the glyph from the bitmap using the context for colors
  bagl_hal_draw_bitmap_within_rect(component->x, 
                                   component->y, 
                                   icon_details->width, 
                                   icon_details->height, 
                                   1<<(icon_details->bpp),
                                   icon_details->colors,
                                   icon_details->bpp, 
                                   icon_details->bitmap,
                                   icon_details->bpp*(icon_details->width*icon_details->height));
}

// --------------------------------------------------------------------------------------

void bagl_draw(const bagl_component_t* component) {
  // component without text
  bagl_draw_with_context(component, NULL, 0, 0);
}
