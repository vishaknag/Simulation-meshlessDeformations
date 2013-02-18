/* Header: world
 * Description: Header file for rendering background and/or environment objects.
 */

#ifndef _WORLD_H_
#define _WORLD_H_

#define CORNELLSIZE 2.0

void worldInit();
void renderAxis();
void setBoundingBox(struct boundBox * box);
void renderWireBox();
void renderCornellBox();
void changeBox(int index);
#endif
