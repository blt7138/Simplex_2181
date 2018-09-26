#include "AppClass.h"
std::vector<MyMesh*> cubes; //MAKE SURE YOU DELETE IN RELEASE
std::vector<vector3> positions;
float i = 0;
void Application::InitVariables(void)
{
	//Make MyMesh object
	for (size_t i = 0; i < 46; i++)
	{
		m_pMesh = new MyMesh();
		m_pMesh->GenerateCube(1.0f, C_BLACK);
		cubes.push_back(m_pMesh);
	}
	
	//variables to help center the alien
	float xOffset = -7.0f;
	float yOffset = -3.0f;

	//Bottom row (row 0)
	positions.push_back(vector3(3.0f + xOffset, 0.0f + yOffset, 0.0f));
	positions.push_back(vector3(4.0f + xOffset, 0.0f + yOffset, 0.0f));
	positions.push_back(vector3(6.0f + xOffset, 0.0f + yOffset, 0.0f));
	positions.push_back(vector3(7.0f + xOffset, 0.0f + yOffset, 0.0f));

	//Row 1
	positions.push_back(vector3(0.0f + xOffset, 1.0f + yOffset, 0.0f));
	positions.push_back(vector3(2.0f + xOffset, 1.0f + yOffset, 0.0f));
	positions.push_back(vector3(8.0f + xOffset, 1.0f + yOffset, 0.0f));
	positions.push_back(vector3(10.0f + xOffset, 1.0f + yOffset, 0.0f));

	//Row 2
	for (float i = 0.0f; i < 11.0f; i++) //Row 2 is nearly full, so a for loop is easier
	{
		if (i != 1.0f && i != 9.0f) //Exclude the two missing boxes
		{
			positions.push_back(vector3(i + xOffset, 2.0f + yOffset, 0.0f));
		}
	}

	//Row 3
	for (float i = 0.0f; i < 11.0f; i++) //The entire row is full
	{
		positions.push_back(vector3(i + xOffset, 3.0f + yOffset, 0.0f));
	}

	//Row 4
	for (float i = 0.0f; i < 11.0f; i++)
	{
		if (i != 0.0f && i != 3.0f && i != 7.0 && i != 10.0)
		{
			positions.push_back(vector3(i + xOffset, 4.0f + yOffset, 0.0f));
		}
	}

	//Row 5
	for (float i = 0; i < 11.0f; i++)
	{
		if (i != 0.0f && i != 1.0f && i != 9.0 && i != 10.0)
		{
			positions.push_back(vector3(i + xOffset, 5.0f + yOffset, 0.0f));
		}
	}

	//Row 6
	positions.push_back(vector3(3.0f + xOffset, 6.0f + yOffset, 0.0f));
	positions.push_back(vector3(7.0f + xOffset, 6.0f + yOffset, 0.0f));

	//Row 7
	positions.push_back(vector3(2.0f + xOffset, 7.0f + yOffset, 0.0f));
	positions.push_back(vector3(8.0f + xOffset, 7.0f + yOffset, 0.0f));
			
	m_pCameraMngr->SetPositionTargetAndUpward(vector3(0.0f, 0.0f, 10.0f), vector3(0.0f), AXIS_Y);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();
	int vectorIndex = 0;
	for (MyMesh* mesh : cubes)
	{
		matrix4 m4Translation = glm::translate(vector3(i, 0.0f, 0.0f) + positions[vectorIndex]); //adding a position from the position vector moves a box to each of those spaces
		mesh->Render(m_pCameraMngr->GetProjectionMatrix(), m_pCameraMngr->GetViewMatrix(), m4Translation);
		i += 0.0001;
		vectorIndex++;
	}	
		
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	for (MyMesh* mesh : cubes)
	{
		if (mesh != nullptr)
		{
			delete mesh;
			mesh = nullptr;
		}
	}
	//SafeDelete(m_pMesh1);
	//release GUI
	ShutdownGUI();
}