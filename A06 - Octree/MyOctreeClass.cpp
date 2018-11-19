#include "MyOctreeClass.h"

uint MyOctreeClass::m_uOctantCount = 0;
uint MyOctreeClass::m_uMaxLevel = 3;
uint MyOctreeClass::m_uIdealEntityCount = 5;

MyOctreeClass::MyOctreeClass(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//Set values
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax;
	//Get the max and min of each entity in the scene
	uint nObjects = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < nObjects; i++)
	{
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();
		lMinMax.push_back(pRigidBody->GetMinGlobal());
		lMinMax.push_back(pRigidBody->GetMaxGlobal());
	}
	//Create a box based on the entities inside of it
	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	vector3 vHalfWidth = pRigidBody->GetHalfWidth();
	float fMax = vHalfWidth.x;
	for (int i = 1; i < 3; i++)
	{
		if (fMax < vHalfWidth[i])
			fMax = vHalfWidth[i];
	}
	vector3 v3Center = pRigidBody->GetCenterGlobal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	//Set the dimensions of the box
	m_fSize = fMax * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;
	
	ConstructTree(m_uMaxLevel);
}

MyOctreeClass::MyOctreeClass(vector3 a_v3Center, float a_fSize)
{
	//Set values
	Init();
	//Set the dimensions of the box
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

MyOctreeClass::MyOctreeClass(MyOctreeClass const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot, other.m_pRoot;
	m_lChild, other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
}

void MyOctreeClass::Init()
{
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (uint n = 0; n < 8; n++)
	{
		m_pChild[n] = nullptr;
	}
}

MyOctreeClass& MyOctreeClass::operator=(MyOctreeClass const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctreeClass temp(other);
		Swap(temp);
	}
	return *this;
}

MyOctreeClass::~MyOctreeClass(void)
{
	Release();
}

void MyOctreeClass::Release()
{
	if (m_uLevel == 0)
	{
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	ClearEntityList();
	m_lChild.clear();
}

void MyOctreeClass::Swap(MyOctreeClass & other)
{
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (uint i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}

float MyOctreeClass::GetSize(void)
{
	return m_fSize;
}

vector3 MyOctreeClass::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 MyOctreeClass::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 MyOctreeClass::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool MyOctreeClass::IsColliding(uint a_uRBIndex)
{
	//Find the entity and rigid body of the given index in the scene
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	if (a_uRBIndex >= nObjectCount)
		return false;
	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3MinO = pRigidBody->GetMinGlobal();
	vector3 v3MaxO = pRigidBody->GetMaxGlobal();

	//Compare the max and mins of the boxes to determine if they overlap and therefore there is a collision
	if (m_v3Max.x < v3MinO.x)
		return false;
	if (m_v3Min.x > v3MaxO.x)
		return false;

	if (m_v3Max.y < v3MinO.y)
		return false;
	if (m_v3Min.y > v3MaxO.y)
		return false;

	if (m_v3Max.z < v3MinO.z)
		return false;
	if (m_v3Min.z > v3MaxO.z)
	return false;

	return true;
}

void MyOctreeClass::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
			glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);

		return;
	}
	for (uint nIndex = 0; nIndex < m_uChildren; nIndex++)
	{
		m_pChild[nIndex]->Display(a_nIndex);
	}
}

void MyOctreeClass::Display(vector3 a_v3Color)
{
	for (uint nIndex = 0; nIndex < m_uChildren; nIndex++)
	{
		m_pChild[nIndex]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctreeClass::DisplayLeafs(vector3 a_v3Color)
{
	uint nLeafs = m_lChild.size();
	for (uint nChild = 0; nChild < nLeafs; nChild++)
	{
		m_lChild[nChild]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctreeClass::ClearEntityList(void)
{
	for (uint nChild = 0; nChild < m_uChildren; nChild++)
	{
		m_pChild[nChild]->ClearEntityList();
	}
	m_EntityList.clear();
}

void MyOctreeClass::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel) //Nothing to subdivide
		return;

	if (m_uChildren != 0) //No children to subdivide into
		return;

	m_uChildren = 8;

	//Set the subdivided size
	float fSize = m_fSize / 4.0f;
	float fSizeD = fSize * 2.0f;
	vector3 v3Center;

	//Position each octant relative to the center of the current octant
	v3Center = m_v3Center;
	v3Center.x -= fSize;
	v3Center.y -= fSize;
	v3Center.z -= fSize;
	m_pChild[0] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.x += fSizeD;
	m_pChild[1] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.z += fSizeD;
	m_pChild[2] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.x -= fSizeD;
	m_pChild[3] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.y += fSizeD;
	m_pChild[4] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.z -= fSizeD;
	m_pChild[5] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.x += fSizeD;
	m_pChild[6] = new MyOctreeClass(v3Center, fSizeD);

	v3Center.z += fSizeD;
	m_pChild[7] = new MyOctreeClass(v3Center, fSizeD);

	for (uint nIndex = 0; nIndex < 8; nIndex++)
	{
		m_pChild[nIndex]->m_pRoot = m_pRoot;
		m_pChild[nIndex]->m_pParent = this;
		m_pChild[nIndex]->m_uLevel = m_uLevel + 1;
		if (m_pChild[nIndex]->ContainsMoreThan(m_uIdealEntityCount)) //Too many entities inside it, keep subdividing
		{
			m_pChild[nIndex]->Subdivide();
		}
	}
}

MyOctreeClass * MyOctreeClass::GetChild(uint a_nChild)
{
	if (a_nChild > 7) return nullptr;
	return m_pChild[a_nChild];
}

MyOctreeClass * MyOctreeClass::GetParent(void)
{
	return m_pParent;
}

bool MyOctreeClass::IsLeaf(void)
{
	return m_uChildren == 0;
}

bool MyOctreeClass::ContainsMoreThan(uint a_nEntities)
{
	uint nCount = 0;
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	for (uint n = 0; n < nObjectCount; n++)
	{
		if (IsColliding(n)) //The current octant will collide with the entities if they are inside it
			nCount++;
		if (nCount > a_nEntities)
			return true;
	}
	return false;
}

void MyOctreeClass::KillBranches(void)
{
	for (uint nIndex = 0; nIndex < m_uChildren; nIndex++)
	{
		m_pChild[nIndex]->KillBranches(); //Kill all of the branches of all of the children until they have no more children as well
		delete m_pChild[nIndex];
		m_pChild[nIndex] = nullptr;
	}
	m_uChildren = 0;
}

void MyOctreeClass::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0)
		return;

	m_uMaxLevel = a_nMaxLevel;

	m_uOctantCount = 1;

	ClearEntityList();

	KillBranches();
	m_lChild.clear();

	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	AssignIDtoEntity();

	ConstructList();
}

void MyOctreeClass::ConstructList(void)
{
	for (uint nChild = 0; nChild < m_uChildren; nChild++)
	{
		m_pChild[nChild]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}

void MyOctreeClass::AssignIDtoEntity(void)
{
	for (uint nChild = 0; nChild < m_uChildren; nChild++)
	{
		m_pChild[nChild]->AssignIDtoEntity();
	}
	if (m_uChildren == 0) //Finally reached an octant with no children
	{
		uint nEntities = m_pEntityMngr->GetEntityCount();
		for (uint nIndex = 0; nIndex < nEntities; nIndex++)
		{
			if (IsColliding(nIndex))
			{
				m_EntityList.push_back(nIndex);
				m_pEntityMngr->AddDimension(nIndex, m_uID); //The entity exists in the same dimension as the current octant being checked
			}
		}
	}
}

uint MyOctreeClass::GetOctantCount(void)
{
	return m_uOctantCount;
}
