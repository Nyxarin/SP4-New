/**
 CWeaponInfo
 By: Toh Da Jun
 Date: Sep 2020
 */
#include "WeaponInfo.h"

// Include CShaderManager
#include "RenderControl/ShaderManager.h"

// Include CProjectile
#include "Projectile.h"

// Include CProjectileManager
#include "ProjectileManager.h"

#include <iostream>
using namespace std;

/**
@brief Default Constructor
*/
CWeaponInfo::CWeaponInfo()
	: dTimeBetweenShots(0.5)
	, dElapsedTime(0.0)
	, bFire(true)
{
}

/**
 @brief Default Destructor
 */
CWeaponInfo::~CWeaponInfo()
{
}

/**
 @brief Set the time between shots
 @param dTimeBetweenShots A const double variable containing the time between shots
 */
void CWeaponInfo::SetTimeBetweenShots(const double dTimeBetweenShots)
{
	this->dTimeBetweenShots = dTimeBetweenShots;
}

/**
 @brief Set the firing rate in rounds per min
 @param firingRate A const int variable containing the firing rate for this weapon
 */
void CWeaponInfo::SetFiringRate(const int firingRate)
{
	dTimeBetweenShots = 60.0 / (double)firingRate;	// 60 seconds divided by firing rate
}

/**
 @brief Set the firing flag
 @param bFire A const bool variable describing if this weapon can fire
 */
void CWeaponInfo::SetCanFire(const bool bFire)
{
	this->bFire = bFire;
}

/**
 @brief Get the time between shots
 @return A double variable
 */
double CWeaponInfo::GetTimeBetweenShots(void) const
{
	return dTimeBetweenShots;
}

/**
 @brief Get the firing rate
 @return An int variable
 */
int CWeaponInfo::GetFiringRate(void) const
{
	return (int)(60.0 / dTimeBetweenShots);	// 60 seconds divided by dTimeBetweenShots
}

/**
 @brief Get the firing flag
 @return A bool variable
 */
bool CWeaponInfo::GetCanFire(void) const
{
	return bFire;
}

/**
 @brief Initialise this instance to default values
 @return A bool variable
 */
bool CWeaponInfo::Init(void)
{
	// The time between shots
	dTimeBetweenShots = 0.5f;
	// The elapsed time (between shots)
	dElapsedTime = dTimeBetweenShots;
	// Boolean flag to indicate if weapon can fire now
	bFire = true;

	// Update the model matrix
	model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	model = glm::translate(model, glm::vec3(vec3Position.x, vec3Position.y, vec3Position.z));
	model = glm::rotate(model, fRotationAngle, vec3RotationAxis);
	model = glm::scale(model, vec3Scale);

	return true;
}

/**
 @brief Update the elapsed time
 @param dt A const double variable containing the elapsed time since the last frame
 @return A bool variable
 */
bool CWeaponInfo::Update(const double dt)
{
	// Update the model matrix
	model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	model = glm::translate(model, glm::vec3(vec3Position.x, vec3Position.y, vec3Position.z));
	model = glm::scale(model, vec3Scale);
	model = glm::rotate(model, fRotationAngle, vec3RotationAxis);


	// If the weapon can fire, then just fire and return
	if (bFire)
		return false;

	// Update the elapsed time if there is no reload to countdown
	if (dElapsedTime > 0.0f)
	{
		dElapsedTime -= dt;
		// Return true since we have already updated the dReloadTime
		return true;
	}
	else
	{
		// Set the weapon to fire ready since reloading is completed
		bFire = true;
	}

	return false;
}

/**
 @brief Discharge this weapon
 @param vec3Position A const glm::vec3 variable containing the source position of the projectile
 @param vec3Front A const glm::vec3 variable containing the direction of the projectile
 @param pSource A const CSolidObject* variable containing the source of this projectile
 @return A bool variable
 */
bool CWeaponInfo::Discharge(glm::vec3 vec3Position, 
							glm::vec3 vec3Front, 
							CSolidObject* pSource)
{
	if (bFire)
	{
		// Create a projectile. 
		// Its position is slightly in front of the player to prevent collision
		// Its direction is same as the player.
		// It will last for 2.0 seconds and travel at 20 units per frame
		CProjectileManager::GetInstance()->Activate(vec3Position + vec3Front * 0.75f,
													vec3Front,
													2.0f, 20.0f, pSource);

		// Lock the weapon after this discharge
		bFire = false;
		// Reset the dElapsedTime to dTimeBetweenShots for the next shot
		dElapsedTime = dTimeBetweenShots;

		return true;
	}

	return false;
}

/**
 @brief Set model
 @param model A glm::mat4 variable to be assigned to this class instance
 */
void CWeaponInfo::SetModel(glm::mat4 model)
{
	this->model = model;
}

/**
 @brief Set view
 @param view A glm::mat4 variable to be assigned to this class instance
 */
void CWeaponInfo::SetView(glm::mat4 view)
{
	this->view = view;
}

/**
 @brief Set projection
 @param projection A glm::mat4 variable to be assigned to this class instance
 */
void CWeaponInfo::SetProjection(glm::mat4 projection)
{
	this->projection = projection;
}

/**
 @brief Set up the OpenGL display environment before rendering
 */
void CWeaponInfo::PreRender(void)
{
	// Change depth function so depth test passes when values are equal to depth buffer's content
	glDepthFunc(GL_LEQUAL);

	// Activate normal 3D shader
	CShaderManager::GetInstance()->Use(sShaderName);
}

/**
 @brief Render this instance
 */
void CWeaponInfo::Render(void)
{	
	// note: currently we set the projection matrix each frame, but since the projection 
	// matrix rarely changes it's often best practice to set it outside the main loop only once.
	CShaderManager::GetInstance()->activeShader->setMat4("projection", projection);
	CShaderManager::GetInstance()->activeShader->setMat4("view", view);
	CShaderManager::GetInstance()->activeShader->setMat4("model", model);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, iTextureID);
		// Render the mesh
		glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, iIndicesSize, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CWeaponInfo::PostRender(void)
{
	glDepthFunc(GL_LESS); // set depth function back to default
}

/**
 @brief Print Self
 */
void CWeaponInfo::PrintSelf(void)
{
	cout << "CWeaponInfo::PrintSelf()" << endl;
	cout << "========================" << endl;
	cout << "dTimeBetweenShots\t:\t" << dTimeBetweenShots << endl;
	cout << "dElapsedTime\t\t:\t" << dElapsedTime << endl;
	cout << "bFire\t\t:\t" << bFire << endl;
}
