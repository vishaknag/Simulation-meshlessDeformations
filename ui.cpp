#include "render.h"
#include "ui.h"
#include "camera.h"

/* Function: control_cb
 * Description: Callback function for user input in GLUI window.
 * Input: control - Control ID
 * Output: None
 */
void control_cb(int control)
{
	pModel *temp;

	switch (control)
	{
		case MODE:
			if (iMouseModel != -1)
			{
				temp = phyzxModels;
				while(temp->next != NULL)
				{
					if(temp->mIndex == iMouseModel)
					{
						setGlobal(temp->pObj);
						break;
					}
					temp = temp->next;
				}
			} //end if
			break;
		case RESET:
			reset();
			break;
		case PAUSE:
			pause = 1 - pause;
			break;
		case CAMERA:
			camInit();
			break;
		case TSTEP:
			if (gTStep < 0.0)
				gTStep = 0.001;
			temp = phyzxModels;
			while(temp->next != NULL)
			{
				temp->pObj->h = gTStep;
				temp = temp->next;
			}
			break;
		case NSTEP:
			if (gNStep < 1)
				gNStep = 1;
			temp = phyzxModels;
			while(temp->next != NULL)
			{
				temp->pObj->n = gNStep;
				temp = temp->next;
			}
			break;
		case KCOL:
			if (iMouseModel != -1)
			{
				temp = phyzxModels;
				while(temp->next != NULL)
				{
					if(temp->mIndex == iMouseModel)
					{
						setGlobal(temp->pObj);
						break;
					}
					temp = temp->next;
				}
			} //end if
			break;
		case KDAMP:
			if (iMouseModel != -1)
			{
				temp = phyzxModels;
				while(temp->next != NULL)
				{
					if(temp->mIndex == iMouseModel)
					{
						setGlobal(temp->pObj);
						break;
					}
					temp = temp->next;
				}
			} //end if
			break;
		case ALPHAFACTOR:
			if (gAlpha < 0.0)
				gAlpha = 0.0;
			else if (gAlpha > 1.0)
				gAlpha = 1.0;
			if (iMouseModel != -1)
			{
				temp = phyzxModels;
				while(temp->next != NULL)
				{
					if(temp->mIndex == iMouseModel)
					{
						setGlobal(temp->pObj);
						break;
					}
					temp = temp->next;
				}
			} //end if
			break;
		case BETAFACTOR:
			if (gBeta < 0.0)
				gBeta = 0.0;
			else if (gBeta > 1.0)
				gBeta = 1.0;
			if (iMouseModel != -1)
			{
				temp = phyzxModels;
				while(temp->next != NULL)
				{
					if(temp->mIndex == iMouseModel)
					{
						setGlobal(temp->pObj);
						break;
					}
					temp = temp->next;
				}
			} //end if
			break;
		case DELTAFACTOR:
			if (gDelta < 0.0)
				gDelta = 0.0;
			else if (gDelta > 1.0)
				gDelta = 1.0;
			if (iMouseModel != -1)
			{
				temp = phyzxModels;
				while(temp->next != NULL)
				{
					if(temp->mIndex == iMouseModel)
					{
						setGlobal(temp->pObj);
						break;
					}
					temp = temp->next;
				}
			} //end if
			break;
		case GVITY:
			if (gGravity < -10.0)
				gGravity = -10.0;
			else if (gGravity > 10.0)
				gGravity = 10.0;
			break;
		case NEXTMODEL:
			if(gNextModelID == 0)
				strcpy(gCrateName, "crateFragile.ppm");
			else if(gNextModelID == 1)
				strcpy(gCrateName, "cratePlus.ppm");
			else if(gNextModelID == 2)
				strcpy(gCrateName, "crateAmmunition.ppm");
			else if(gNextModelID == 3)
				strcpy(gCrateName, "crateCrossbar.ppm");
			break;
		case ADDMODEL:
			if(gNextModelID == 4)
				strcpy(filename, "bunny.obj");
			else
				strcpy(filename, "crate.obj");
			AddModel(filename, RANDOMPOS);
			break;
		case TESTCASE:
			if(gNextModelID == 4)
				strcpy(filename, "bunny.obj");
			else
				strcpy(filename, "crate.obj");

			RunTestCase();
			break;
		case FRAMERATE:
			gFRateON = 1 - gFRateON;
			break;
	} //end switch

	glui->sync_live();
}

/* Function: setGlobal
 * Description: Applies the user input into the physics calculations.
 * Input: phyzxObj - Physics information for an object
 * Output: None
 */
void setGlobal(phyzx *phyzxObj)
{
	phyzxObj->h = gTStep;
	phyzxObj->n = gNStep;
	phyzxObj->kWall = gKCol;
	phyzxObj->dWall = gDCol;
	phyzxObj->alpha = gAlpha;
	phyzxObj->beta = gBeta;
	phyzxObj->delta = gDelta;
	phyzxObj->deformMode = gDeformMode;
} //end setGlobal

/* Function: dispPhysics
 * Description: Displays the physics information of an object on the GLUI.
 * Input: phyzxObj - Physics information for an object
 * Output: None
 */
void dispPhysics(phyzx *phyzxObj)
{
	gTStep = phyzxObj->h;
	gNStep = phyzxObj->n;
	gKCol = phyzxObj->kWall;
	gDCol = phyzxObj->dWall;
	gAlpha = phyzxObj->alpha;
	gBeta = phyzxObj->beta;
	gDelta = phyzxObj->delta;
	gDeformMode = phyzxObj->deformMode;

	glui->sync_live();
} //end dispPhysics

/* Function: createGLUI
 * Description: Creates and initializes the GLUI window.
 * Input: None
 * Output: None
 */
void createGLUI()
{
	// Create a GLUI window
	glutInitWindowPosition(WINRESX + 20,0);
	glui = GLUI_Master.create_glui("controls", 0, 600, -1);
	//GLUI *glui = GLUI_Master.create_glui_subwindow( mainWindowId, GLUI_SUBWINDOW_BOTTOM ); 

    glui->set_main_gfx_window( mainWindowId );

	GLUI_Panel *main_panel = glui->add_panel ( "Meshless Deformation Controls" );

		GLUI_Panel *lights_panel = glui->add_panel_to_panel( main_panel, "LIGHTS", 1 );

			glui->add_statictext_to_panel( lights_panel, "ON / OFF" );
			glui->add_checkbox_to_panel( lights_panel, "Light 1", &light1 );
			glui->add_checkbox_to_panel( lights_panel, "Light 2", &light2 );
			glui->add_checkbox_to_panel( lights_panel, "Light 3", &light3 );
			glui->add_checkbox_to_panel( lights_panel, "Light 4", &light4 );
			glui->add_checkbox_to_panel( lights_panel, "Light 5", &light5 );
			glui->add_checkbox_to_panel( lights_panel, "Light 6", &light6 );
			glui->add_checkbox_to_panel( lights_panel, "Light 7", &light7 );
			glui->add_checkbox_to_panel( lights_panel, "Light 8", &light8 );

			glui->add_column_to_panel(lights_panel, true);

			glui->add_statictext_to_panel( lights_panel, "EFFECTS" );
			glui->add_checkbox_to_panel( lights_panel, "Shininess", &setShineLevel );
			glui->add_checkbox_to_panel( lights_panel, "Specularity", &setSpecLevel );
			glui->add_checkbox_to_panel( lights_panel, "Emissivity", &setEmissLevel );
			glui->add_separator_to_panel(lights_panel);
			
			glui->add_statictext_to_panel(lights_panel, "MATERIAL COLORS" );
			GLUI_RadioGroup *matColor =	glui->add_radiogroup_to_panel(lights_panel, &materialColor);
			glui->add_radiobutton_to_group( matColor, "Gold" );
			glui->add_radiobutton_to_group( matColor, "Maroon" );
			glui->add_radiobutton_to_group( matColor, "flesh" );

			glui->add_separator_to_panel(main_panel);
		
			GLUI_Panel *environ_panel = glui->add_panel_to_panel( main_panel, "ENVIRONMENT", 1 );
			glui->add_statictext_to_panel( environ_panel, "Bounding Box");
			GLUI_RadioGroup *environMat =	glui->add_radiogroup_to_panel(environ_panel, &boxType);
			glui->add_radiobutton_to_group( environMat, "None" );
			glui->add_radiobutton_to_group( environMat, "Wire Frame" );
			glui->add_radiobutton_to_group( environMat, "Cornell Box" );
			glui->add_radiobutton_to_group( environMat, "Sky Box" );

			glui->add_column_to_panel(environ_panel, true);

			glui->add_checkbox_to_panel( environ_panel, "World Axis", &axis );
				glui->add_checkbox_to_panel( environ_panel, "Sticky Floor", &stickyFloor );
			
			glui->add_separator_to_panel(main_panel);

			GLUI_Panel *parameters_panel = glui->add_panel_to_panel( main_panel, "LIVE", 1 );
				GLUI_EditText *tStepParam = glui->add_edittext_to_panel( parameters_panel, "TSTEP", GLUI_EDITTEXT_FLOAT, &gTStep, TSTEP, control_cb);
				GLUI_EditText *nStepParam = glui->add_edittext_to_panel( parameters_panel, "NSTEP", GLUI_EDITTEXT_INT, &gNStep, NSTEP, control_cb);
				GLUI_EditText *kCollisionParam = glui->add_edittext_to_panel( parameters_panel, "KCOLL", GLUI_EDITTEXT_FLOAT, &gKCol, KCOL, control_cb);
				GLUI_EditText *kDampParam = glui->add_edittext_to_panel( parameters_panel, "KDAMP", GLUI_EDITTEXT_FLOAT, &gDCol, KDAMP, control_cb);
				GLUI_EditText *gravityParam = glui->add_edittext_to_panel( parameters_panel, "GRAVITY", GLUI_EDITTEXT_FLOAT, &gGravity, GVITY, control_cb);
				GLUI_EditText *alphaParam = glui->add_edittext_to_panel( parameters_panel, "ALPHA", GLUI_EDITTEXT_FLOAT, &gAlpha, ALPHAFACTOR, control_cb);
				GLUI_EditText *betaParam = glui->add_edittext_to_panel( parameters_panel, "BETA", GLUI_EDITTEXT_FLOAT, &gBeta, BETAFACTOR, control_cb);
				GLUI_EditText *deltaParam = glui->add_edittext_to_panel( parameters_panel, "DELTA", GLUI_EDITTEXT_FLOAT, &gDelta, DELTAFACTOR, control_cb);

			glui->add_column_to_panel(parameters_panel, true);

			glui->add_statictext_to_panel(parameters_panel, "SELECT CRATE" );
			GLUI_RadioGroup *nextModel = glui->add_radiogroup_to_panel(parameters_panel, &gNextModelID, NEXTMODEL, control_cb);
			glui->add_radiobutton_to_group( nextModel, "Fragile" );
			glui->add_radiobutton_to_group( nextModel, "Medical" );
			glui->add_radiobutton_to_group( nextModel, "Ammunition" );
			glui->add_radiobutton_to_group( nextModel, "Plain" );
			glui->add_radiobutton_to_group( nextModel, "Bunny" );

			glui->add_column_to_panel(main_panel, true);

			GLUI_Panel *mouse_panel = glui->add_panel_to_panel( main_panel, "LEFT MOUSE CONTROLS", 1 );
			
			GLUI_RadioGroup *mouseLeft = glui->add_radiogroup_to_panel(mouse_panel, &lMouseVal);
			glui->add_radiobutton_to_group( mouseLeft, "None" );
			glui->add_radiobutton_to_group( mouseLeft, "Model" );
			glui->add_radiobutton_to_group( mouseLeft, "Model Vertices" );

			GLUI_Panel *mode_panel = glui->add_panel_to_panel( main_panel, "MODES", 1 );
			GLUI_RadioGroup *modeSelection = glui->add_radiogroup_to_panel(mode_panel, &gDeformMode, MODE, control_cb);
			glui->add_radiobutton_to_group( modeSelection, "Shape Matching" );
			glui->add_radiobutton_to_group( modeSelection, "Rigid Body Deformation" );
			glui->add_radiobutton_to_group( modeSelection, "Linear Deformation" );
			glui->add_radiobutton_to_group( modeSelection, "Quadratic Deformation" );

			GLUI_Panel *camera_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *cameraButton = glui->add_button_to_panel( camera_panel, "RESET CAMERA", CAMERA, control_cb);

			GLUI_Panel *reset_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *resetButton = glui->add_button_to_panel( reset_panel, "RESET", RESET, control_cb);

			GLUI_Panel *pause_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *pauseButton = glui->add_button_to_panel( pause_panel, "PAUSE", PAUSE, control_cb);
			
			GLUI_Panel *addmodel_panel = glui->add_panel_to_panel( main_panel, "", 1 );
			GLUI_Button *addmodelButton = glui->add_button_to_panel( addmodel_panel, "ADD MODEL", ADDMODEL, control_cb);

			GLUI_Panel *testcase_panel = glui->add_panel_to_panel( main_panel, "SM vs RIGID", 1 );
			GLUI_Button *runTestCaseButton = glui->add_button_to_panel( testcase_panel, "RUN TEST CASE", TESTCASE, control_cb);

			GLUI_Panel *framerate_panel = glui->add_panel_to_panel( main_panel, "FRAMERATE", 1 );
			GLUI_Button *framerateCaseButton = glui->add_button_to_panel( framerate_panel, "ON / OFF", FRAMERATE, control_cb);

	glui->set_main_gfx_window( mainWindowId );
} //end createGLUI
