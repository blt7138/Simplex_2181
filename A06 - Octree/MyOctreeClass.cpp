#include "MyOctreeClass.h"

uint MyOctreeClass::m_uOctantCount = 0;
uint MyOctreeClass::m_uMaxLevel = 3;
uint MyOctreeClass::m_uIdealEntityCount = 5;

MyOctreeClass::MyOctreeClass(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
}

MyOctreeClass::MyOctreeClass(vector3 a_v3Center, float a_fSize)
{
}

MyOctreeClass::MyOctreeClass(MyOctreeClass const & other)
{
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
	m_pEntityMngr = EntityManager::GetInstance();

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
	m_pEntityMngr = EntityManager::GetInstance();

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
	return false;
}

void MyOctreeClass::Display(uint a_nIndex, vector3 a_v3Color)
{
}

void MyOctreeClass::Display(vector3 a_v3Color)
{
}

void MyOctreeClass::DisplayLeafs(vector3 a_v3Color)
{
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
		if (IsColliding(n))
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
		m_pChild[nIndex]->KillBranches();
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
	if (m_uChildren == 0)
	{
		uint nEntities = m_pEntityMngr->GetEntityCount();
		for (uint nIndex = 0; nIndex < nEntities; nIndex++)
		{
			if (IsColliding(nIndex))
			{
				m_EntityList.push_back(nIndex);
				m_pEntityMngr->AddDimension(nIndex, m_uID);
			}
		}
	}
}

uint MyOctreeClass::GetOctantCount(void)
{
	return m_uOctantCount;
}
