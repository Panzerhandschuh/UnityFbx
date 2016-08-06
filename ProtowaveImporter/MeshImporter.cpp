#include "stdafx.h"
#include "MeshImporter.h"

using namespace std;
using namespace boost::filesystem;

MeshImporter::MeshImporter(const path &inputFile)
{
	manager = FbxManager::Create();
	FbxIOSettings *ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	FbxImporter *importer = FbxImporter::Create(manager, "");

	if (!importer->Initialize(inputFile.string().c_str(), -1, manager->GetIOSettings()))
		throw runtime_error("Failed to initialize fbx importer");

	scene = FbxScene::Create(manager, "scene");
	importer->Import(scene); // Creates a .fbm folder in the inputFile directory and imports the scene
	importer->Destroy();
}

MeshImporter::~MeshImporter()
{
	manager->Destroy();
}

bool MeshImporter::Import(Mesh &mesh, bool isCollider)
{
	this->isCollider = isCollider;

	// Triangulate scene
	FbxGeometryConverter converter(manager);
	if (!converter.Triangulate(scene, true))
	{
		cerr << "Import Error: Failed to triangulate scene" << endl;
		return false;
	}

	// Get meshes
	FbxArray<FbxNode*> fbxMeshes;
	GetAllMeshes(scene->GetRootNode(), fbxMeshes);
	if (fbxMeshes.Size() == 0)
	{
		cerr << "Import Error: No mesh objects detected" << endl;
		return false;
	}
	//else
	//	cout << "Found " << fbxMeshes.Size() << " mesh(es) in the fbx file" << endl;

	// Merge meshes
	FbxNode* fbxNode = converter.MergeMeshes(fbxMeshes, "Mesh", scene);
	if (!fbxNode)
	{
		cerr << "Import Error: Failed to merge meshes" << endl;
		return false;
	}

	FbxMesh *fbxMesh = (FbxMesh*)fbxNode->GetNodeAttribute();

	//ProcessNode(scene->GetRootNode(), meshes, -1);
	mesh = ProcessMesh(fbxMesh, fbxMeshes);

	return true;
}

//void MeshImporter::ProcessNode(FbxNode *node, vector<Mesh> &meshes, int parentIndex)
//{
//	FbxNodeAttribute *attribute = node->GetNodeAttribute();
//	if (attribute)
//	{
//		FbxNodeAttribute::EType attributeType = attribute->GetAttributeType();
//		if (attributeType == FbxNodeAttribute::eMesh)
//		{
//			FbxMesh *fbxMesh = node->GetMesh();
//			Mesh mesh = ProcessMesh(fbxMesh);
//			mesh.parentIndex = parentIndex;
//			parentIndex++;
//			meshes.push_back(mesh);
//		}
//	}
//
//	for (int i = 0; i < node->GetChildCount(); i++)
//		ProcessNode(node->GetChild(i), meshes, parentIndex);
//}

Mesh MeshImporter::ProcessMesh(FbxMesh *fbxMesh, FbxArray<FbxNode*> fbxMeshes)
{
	// Get transform matrices
	FbxNode *node = fbxMesh->GetNode();
	//FbxAMatrix globalTransform = node->EvaluateGlobalTransform();
	////FbxAMatrix lclTransform = node->EvaluateLocalTransform();
	//FbxAMatrix geometry = FbxUtil::GetGeometry(node);
	//FbxAMatrix transform = globalTransform * geometry;

	//mesh.name = node->GetName();
	//mesh.translation = transform.GetT();
	//mesh.rotation = transform.GetR();
	//mesh.scale = transform.GetS();

	// Loop through triangle indices to find unique vertex normals
	// If a unique vertex normal exists, duplicate the vertex
	FbxVector4 *fbxVertices = fbxMesh->GetControlPoints();
	int *fbxTriangles = fbxMesh->GetPolygonVertices();

	FbxArray<FbxVector4> fbxNormals;
	bool hasNormals = (fbxMesh->GetElementNormalCount() > 0);
	if (hasNormals)
		fbxMesh->GetPolygonVertexNormals(fbxNormals);

	FbxArray<FbxVector2> fbxUvs;
	bool hasUvs = (fbxMesh->GetElementUVCount() > 0);
	if (hasUvs)
	{
		//FbxStringList uvNames;
		//fbxMesh->GetUVSetNames(uvNames);
		//fbxMesh->GetPolygonVertexUVs(uvNames[0], fbxUvs);

		// Get UVs from all meshes (since MergeMeshes breaks uvs)
		for (int i = 0; i < fbxMeshes.Size(); i++)
		{
			FbxMesh *tempFbxMesh = (FbxMesh*)fbxMeshes[i]->GetNodeAttribute();

			FbxStringList uvNames;
			tempFbxMesh->GetUVSetNames(uvNames);
			FbxArray<FbxVector2> tempFbxUvs;
			tempFbxMesh->GetPolygonVertexUVs(uvNames[0], tempFbxUvs);

			fbxUvs.AddArray(tempFbxUvs);
		}
	}

	FbxLayerElementArrayTemplate<int> *fbxMaterialIndices = NULL;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	if (fbxMesh->GetElementMaterial())
	{
		fbxMaterialIndices = &fbxMesh->GetElementMaterial()->GetIndexArray();
		materialMappingMode = fbxMesh->GetElementMaterial()->GetMappingMode();
	}

	// Group identical normals and uvs together with their associated vertices
	// If a vertex contains more than one unique normal vector or uv, duplicate it so it can be compatible with Unity engine
	int numControlPoints = fbxMesh->GetControlPointsCount();
	int numPolygons = fbxMesh->GetPolygonCount();
	vector<vector<VertexInfo>> vertexGroups(numControlPoints);
	vector<int> materialIndices(numPolygons);
	int tIndex = 0;
	for (int pIndex = 0; pIndex < numPolygons; pIndex++)
	{
		int matIndex = 0;
		if (fbxMaterialIndices && materialMappingMode == FbxGeometryElement::eByPolygon)
			matIndex = fbxMaterialIndices->GetAt(pIndex);
		materialIndices[pIndex] = matIndex;

		for (int pvIndex = 0; pvIndex < 3; pvIndex++)
		{
			int vIndex = fbxTriangles[tIndex];
			bool foundGroup = false;
			for (int i = 0; i < vertexGroups[vIndex].size(); i++)
			{
				bool duplicateNormal = (!hasNormals || vertexGroups[vIndex][i].normal == fbxNormals[tIndex]);
				bool duplicateUv = (!hasUvs || vertexGroups[vIndex][i].uv == fbxUvs[tIndex]);
				if (duplicateNormal && duplicateUv) // Duplicate normal and uv found
				{
					// Add triangle index to existing group
					vertexGroups[vIndex][i].triangles.push_back(tIndex);
					foundGroup = true;
					break;
				}
			}

			if (!foundGroup)
			{
				// Add a new vertex info
				VertexInfo info;
				if (hasNormals)
					info.normal = fbxNormals[tIndex];
				if (hasUvs)
					info.uv = fbxUvs[tIndex];
				info.triangles.push_back(tIndex);

				vertexGroups[vIndex].push_back(info);
			}

			tIndex++;
		}
	}

	// Construct Unity engine style mesh
	int cvIndex = 0; // Used for tracking vertex indices
	vector<Vector3> vertices;
	vector<Vector3> normals;
	vector<Vector2> uvs;
	vector<int> triangles(numPolygons * 3);
	for (int vIndex = 0; vIndex < numControlPoints; vIndex++)
	{
		vector<VertexInfo> vertexGroup = vertexGroups[vIndex];
		for (int vgIndex = 0; vgIndex < vertexGroup.size(); vgIndex++)
		{
			VertexInfo info = vertexGroup[vgIndex];

			// Add vertex info to unity mesh
			Vector3 vertex = fbxVertices[vIndex];
			vertices.push_back(vertex);

			// Add normal info to unity mesh
			if (hasNormals)
			{
				Vector3 normal = info.normal;
				normals.push_back(normal);
			}

			// Add uv info to unity mesh
			if (hasUvs)
			{
				Vector2 uv = info.uv;
				uvs.push_back(uv);
			}

			// Set triangles
			for (int tIndex = 0; tIndex < info.triangles.size(); tIndex++)
			{
				// Add triangle info to unity mesh
				int itIndex = info.triangles[tIndex];
				triangles[itIndex] = cvIndex;
			}

			cvIndex++;
		}
	}

	// Map material indices to triangles
	int numMaterialIndices = *max_element(materialIndices.begin(), materialIndices.end()) + 1;
	vector<vector<int>> subMeshTriangles(numMaterialIndices);
	for (int i = 0; i < materialIndices.size(); i++)
	{
		int matIndex = materialIndices[i];
		subMeshTriangles[matIndex].push_back(triangles[i * 3]);
		subMeshTriangles[matIndex].push_back(triangles[(i * 3) + 1]);
		subMeshTriangles[matIndex].push_back(triangles[(i * 3) + 2]);
	}

	Mesh mesh;
	mesh.vertices = vertices;
	mesh.normals = normals;
	mesh.uvs = uvs;
	mesh.triangles = triangles;
	mesh.subMeshTriangles = subMeshTriangles;

	if (!isCollider)
	{
		// Import material info
		vector<Material> materials = GetMaterials(node);
		mesh.materials = materials;
	}

	return mesh;
}

void MeshImporter::GetAllMeshes(FbxNode *node, FbxArray<FbxNode*> &fbxMeshes)
{
	FbxNodeAttribute *attribute = node->GetNodeAttribute();
	if (attribute)
	{
		FbxNodeAttribute::EType attributeType = attribute->GetAttributeType();
		if (attributeType == FbxNodeAttribute::eMesh)
			fbxMeshes.Add(node);
	}

	// Get mesh nodes recursively
	for (int i = 0; i < node->GetChildCount(); i++)
		GetAllMeshes(node->GetChild(i), fbxMeshes);
}

vector<Material> MeshImporter::GetMaterials(FbxNode *node)
{
	vector<Material> materials;

	if (node->GetMaterialCount() == 0) // Mesh contains no materials
	{
		//cout << "no mats" << endl;
		// Create default material
		Material mat;
		Material::LoadDefault(mat);
		materials.push_back(mat);
	}
	else // Mesh contains one or more materials
	{
		for (int mIndex = 0; mIndex < node->GetMaterialCount(); mIndex++)
		{
			// Create material properties
			Material mat;
			Material::LoadDefault(mat); // Use default properties in case something goes wrong

			FbxSurfaceMaterial *material = node->GetMaterial(mIndex);
			if (!material)
			{
				cout << "Material Import Warning: Could not get material from mesh node, using default material properties" << endl << endl;
				materials.push_back(mat);
				continue;
			}

			// Get material info
			mat.diffuseMap = GetTexturePath(material, FbxSurfaceMaterial::sDiffuse);
			mat.normalMap = GetTexturePath(material, FbxSurfaceMaterial::sNormalMap);
			mat.specularMap = GetTexturePath(material, FbxSurfaceMaterial::sSpecular);
			if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				FbxSurfacePhong *phong = (FbxSurfacePhong*)material;

				mat.color = phong->Diffuse;

				FbxProperty property = phong->FindProperty("Opacity");
				if (property.IsValid())
				{
					FbxDouble opacity = property.Get<FbxDouble>();
					mat.hasOpacity = opacity < 1;
					if (mat.hasOpacity)
						mat.opacity = opacity;
				}
			}
			else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				FbxSurfaceLambert *lambert = (FbxSurfaceLambert*)material;

				mat.color = lambert->Diffuse;

				FbxProperty property = lambert->FindProperty("Opacity");
				if (property.IsValid())
				{
					FbxDouble opacity = property.Get<FbxDouble>();
					mat.hasOpacity = opacity < 1;
					if (mat.hasOpacity)
						mat.opacity = opacity;
				}
			}
			else
				cout << "Material Import Warning: Unknown material type" << endl;

			// Get uv info
			FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			FbxTexture *texture = property.GetSrcObject<FbxTexture>();
			if (texture)
			{
				mat.uvScaling[0] = texture->GetScaleU();
				mat.uvScaling[1] = texture->GetScaleV();

				mat.uvTranslation[0] = texture->GetTranslationU();
				mat.uvTranslation[1] = texture->GetTranslationV();
			}

			materials.push_back(mat);
		}
	}

	return materials;
}

path MeshImporter::GetTexturePath(FbxSurfaceMaterial *material, const char* propertyName)
{
	FbxProperty property = material->FindProperty(propertyName);
	if (property.IsValid())
	{
		FbxTexture* texture = property.GetSrcObject<FbxTexture>();
		if (texture)
		{
			FbxFileTexture *file = FbxCast<FbxFileTexture>(texture);
			return file->GetFileName();
		}
	}

	return "";
}

FbxVector4 operator*(const FbxVector4 &vector, const FbxAMatrix &matrix)
{
	FbxVector4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i] += matrix[j][i] * vector[j];
		}
	}
	return result;
}

FbxVector4& operator*=(FbxVector4 &vector, const FbxAMatrix &matrix)
{
	FbxVector4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i] += matrix[j][i] * vector[j];
		}
	}
	vector = result;
	return vector;
}