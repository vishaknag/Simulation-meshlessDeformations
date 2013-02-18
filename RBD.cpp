/* Source: RBD
 * Description: Contains functions for Rigid Body Dynamics.
 */

#include "physics.h"

/* Function: defaultDeform
 * Description: Turns off the Rigid Body Dynamics.
 * Input: None
 * Output: None
 */
void defaultDeform(phyzx *phyzxObj)
{
	phyzxObj->alpha = gAlpha;
} //end defaultDeform

/* Function: rigidBody
 * Description: Turns on the Rigid Body Dynamics.
 * Input: None
 * Output: None
 */
void rigidBody(phyzx *phyzxObj)
{
	phyzxObj->alpha = 1.0;
} //end rigidBody