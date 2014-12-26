#include "stdafx.h"
#include "MeshImporter.h"

MeshImporter::MeshImporter(const path &inputFile)
{
	fbxFile = inputFile;
	//materialsDir = materialsDirectory;

	manager = FbxManager::Create();
	FbxIOSettings *ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	FbxImporter *importer = FbxImporter::Create(manager, "");

	if (!importer->Initialize(inputFile.string().c_str(), -1, manager->GetIOSettings()))
	{
		cout << "Failed to initialize fbx importer" << endl;
		cout << "Press enter to continue...";
		cin.ignore();
		exit(EXIT_FAILURE);
	}

	scene = FbxScene::Create(manager, "scene");
	importer->Import(scene); // Creates a .fbm folder in the inputFile directory and imports the scene
	importer->Destroy();

	if (!PrepareMeshes())
	{
		cout << "Press enter to continue...";
		cin.ignore();
		exit(EXIT_FAILURE);
	}
}

MeshImporter::~MeshImporter()
{
	manager->Destroy();
}

bool MeshImporter::Import(Mesh &mesh, bool importMaterials)
{
	FbxArray<FbxNode*> fbxMeshes;
	if (!GetFbxMeshes(fbxMeshes))
		return false;

	// Get total number of triangles
	int numTriangles = 0;
	for (int i = 0; i < fbxMeshes.Size(); i++)
	{
		FbxMesh *fbxMesh = (FbxMesh*)fbxMeshes[i]->GetNodeAttribute();
		numTriangles += fbxMesh->GetPolygonVertexCount();
	}

	vector<Vector3> vertices;
	vector<int> triangles(numTriangles);
	vector<Vector3> normals;
	vector<Vector2> uvs;
	int currentVertexIndex = 0;
	int startTriangleIndex = 0;
	FbxDouble3 centerPivotPoint;

	// Loop through triangle indices to find unique vertex normals
	// If a unique vertex normal exists, duplicate the vertex
	for (int meshIndex = 0; meshIndex < fbxMeshes.Size(); meshIndex++)
	{
		FbxMesh *fbxMesh = (FbxMesh*)fbxMeshes[meshIndex]->GetNodeAttribute();
		FbxVector4 *fbxVertices = fbxMesh->GetControlPoints();
		int numTriangles = fbxMesh->GetPolygonVertexCount();
		int *fbxTriangles = fbxMesh->GetPolygonVertices();

		FbxArray<FbxVector4> fbxNormals;
		fbxMesh->GetPolygonVertexNormals(fbxNormals);

		FbxArray<FbxVector2> fbxUvs;
		bool hasUvs = (fbxMesh->GetElementUVCount() > 0);
		if (hasUvs)
		{
			FbxStringList uvNames;
			fbxMesh->GetUVSetNames(uvNames);
			fbxMesh->GetPolygonVertexUVs(uvNames[0], fbxUvs);
		}
		//cout << "Uv Count: " << fbxUvs.GetCount() << endl;
		//cout << "Polygon Vertex Count: " << fbxMesh->GetPolygonVertexCount() << endl;
		//cout << "Control Point Count: " << fbxMesh->GetControlPointsCount() << endl;
		//FbxGeometryElementMaterial *material = fbxMesh->GetElementMaterial(0);

		// Group identical normals together with their associated vertices
		// If a vertex contains more than one unique normal vector or uv vector, duplicate it so it can be compatible with Unity engine
		int numVertices = fbxMesh->GetControlPointsCount();
		vector<VertexGroupInfo> *vertexNormalGroups = new vector<VertexGroupInfo>[numVertices];

		for (int triangleIndex = 0; triangleIndex < numTriangles; triangleIndex++)
		{
			int vertexIndex = fbxTriangles[triangleIndex];

			bool foundGroup = false;
			int size = vertexNormalGroups[vertexIndex].size();
			for (int i = 0; i < size; i++)
			{
				bool duplicateUv = (!hasUvs || vertexNormalGroups[vertexIndex][i].uv == fbxUvs[triangleIndex]);
				if (vertexNormalGroups[vertexIndex][i].normal == fbxNormals[triangleIndex] && duplicateUv) // Duplicate normal and uv found
				{
					// Add new triangle index to existing group
					vertexNormalGroups[vertexIndex][i].triangleIndices.push_back(triangleIndex);
					foundGroup = true;
				}
			}
			if (!foundGroup)
			{
				// Add a new normal group
				VertexGroupInfo info;
				info.normal = fbxNormals[triangleIndex];
				if (hasUvs)
					info.uv = fbxUvs[triangleIndex];
				info.triangleIndices.push_back(triangleIndex);
				vertexNormalGroups[vertexIndex].push_back(info);
			}
		}

		// Mesh position offset info
		FbxDouble3 meshPosition = fbxMeshes[meshIndex]->LclTranslation.Get();
		if (meshIndex == 0)
			centerPivotPoint = fbxMeshes[meshIndex]->LclTranslation.Get();

		FbxVector4 meshOffset;
		meshOffset[0] = centerPivotPoint[0] - meshPosition[0];
		meshOffset[2] = centerPivotPoint[1] - meshPosition[1]; // The offset index is intentional to account for axis conversion
		meshOffset[1] = centerPivotPoint[2] - meshPosition[2];

		FbxVector4 rotationOffset = fbxMeshes[meshIndex]->LclRotation.Get();
		rotationOffset[0] -= 90;
		std::swap(rotationOffset[1], rotationOffset[2]);

		// Transform matrices
		FbxAMatrix offsetRotationMatrix(FbxVector4(0, 0, 0), FbxVector4(-90, 0, 180), FbxVector4(1, 1, 1));
		meshOffset *= offsetRotationMatrix;
		FbxAMatrix normalRotationMatrix(FbxVector4(0, 0, 0), rotationOffset, FbxVector4(1, 1, 1));
		FbxAMatrix transformMatrix(meshOffset, rotationOffset, FbxVector4(1, 1, 1));

		// Construct Unity engine style mesh
		for (int vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
		{
			int numNormalGroups = vertexNormalGroups[vertexIndex].size();

			for (int normalGroupIndex = 0; normalGroupIndex < numNormalGroups; normalGroupIndex++)
			{
				// Add vertex info to unity mesh
				Vector3 vertex;
				fbxVertices[vertexIndex][3] = 1;
				FbxVector4 transformedVertex = fbxVertices[vertexIndex] * transformMatrix;
				vertex.x = (float)transformedVertex[0];
				vertex.y = (float)transformedVertex[1];
				vertex.z = (float)transformedVertex[2];
				vertices.push_back(vertex);

				// Get triangle indices
				std::vector<int> triangleIndices = vertexNormalGroups[vertexIndex][normalGroupIndex].triangleIndices;
				int numIndices = triangleIndices.size();

				// Add normal info to unity mesh
				Vector3 normal;
				fbxNormals[triangleIndices[0]][3] = 1;
				FbxVector4 transformedNormal = fbxNormals[triangleIndices[0]] * normalRotationMatrix;
				normal.x = (float)transformedNormal[0];
				normal.y = (float)transformedNormal[1];
				normal.z = (float)transformedNormal[2];
				normals.push_back(normal);

				// Add uv info to unity mesh
				if (hasUvs)
				{
					Vector2 uv;
					uv.x = (float)fbxUvs[triangleIndices[0]][0];
					uv.y = -(float)fbxUvs[triangleIndices[0]][1];
					uvs.push_back(uv);
				}

				for (int triangleIndex = 0; triangleIndex < numIndices; triangleIndex++)
				{
					// Add triangle info to unity mesh
					int currentTriangleIndex = triangleIndices[triangleIndex];
					triangles[startTriangleIndex + currentTriangleIndex] = currentVertexIndex;
				}

				currentVertexIndex++;
			}
		}

		startTriangleIndex += fbxMesh->GetPolygonVertexCount();
		delete[] vertexNormalGroups;
	}

	// Flip mesh x axis to work with unity's coordinate system
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		vertices[i].x *= -1;
	}
	for (unsigned int i = 0; i < normals.size(); i++)
	{
		normals[i].x *= -1;
	}
	for (unsigned int i = 0; i < triangles.size(); i += 3) // Flip triangle winding
	{
		int tempIndex = triangles[i];
		triangles[i] = triangles[i + 2];
		triangles[i + 2] = tempIndex;
	}

	mesh.vertices = vertices;
	mesh.triangles = triangles;
	mesh.normals = normals;

	// Import uvs, material ids, and material info
	if (importMaterials)
	{
		vector<Material> materials;
		vector<int> materialIds;
		GetMaterials(fbxMeshes, materials, materialIds);

		mesh.uvs = uvs;
		mesh.materialIds = materialIds;
		mesh.materials = materials;
	}

	return true;
}

bool MeshImporter::PrepareMeshes()
{
	// Find all mesh nodes
	FbxArray<FbxNode*> fbxMeshes;
	if (!GetFbxMeshes(fbxMeshes))
		return false;

	cout << "Found " << fbxMeshes.Size() << " mesh(es) in the fbx file" << endl;

	// Triangulate meshes if they aren't already
	FbxGeometryConverter converter(manager);
	for (int meshIndex = 0; meshIndex < fbxMeshes.Size(); meshIndex++)
	{
		FbxMesh *tempMesh = (FbxMesh*)fbxMeshes[meshIndex]->GetNodeAttribute();
		if (!tempMesh->IsTriangleMesh())
		{
			if (!converter.Triangulate(scene, true, true))
			{
				cout << "Failed to triangulate meshes" << endl;
				return false;
			}
			else
				cout << "Meshes Triangulated" << endl;
			break;
		}
	}
	cout << endl;

	return true;
}

bool MeshImporter::GetFbxMeshes(FbxArray<FbxNode*> &fbxMeshes)
{
	// Get root node
	FbxNode *rootNode = scene->GetRootNode();
	if (!rootNode)
	{
		cout << "Error loading mesh. Fbx scene has no root node" << endl;
		return false;
	}

	for (int i = 0; i < rootNode->GetChildCount(); i++)
	{
		FbxNode *node = rootNode->GetChild(i);
		if (node->GetNodeAttribute() == NULL)
			continue;

		FbxNodeAttribute::EType attribute = node->GetNodeAttribute()->GetAttributeType();
		if (attribute != FbxNodeAttribute::eMesh)
			continue;

		fbxMeshes.Add(node);
	}

	return true;
}

void MeshImporter::GetMaterials(const FbxArray<FbxNode*> &meshes, vector<Material> &materials, vector<int> &materialIndices)
{
	int materialNumber = 0;
	for (int meshIndex = 0; meshIndex < meshes.Size(); meshIndex++)
	{
		FbxMesh *mesh = (FbxMesh*)meshes[meshIndex]->GetNodeAttribute();
		FbxNode *node = mesh->GetNode();
		if (!node)
		{
			cout << "Texture import error: Could not get mesh node." << endl << endl;
			return;
		}
		vector<int> currentMaterialIndices;
		GetMaterialIndices(mesh, currentMaterialIndices);
		if (node->GetMaterialCount() == 0)
		{
			//cout << "no mats" << endl;
			// Create default material
			Material materialProperties;
			FbxDouble diffuseValue = 150.0 / 255.0;
			FbxDouble3 defaultDiffuse(diffuseValue, diffuseValue, diffuseValue);
			materialProperties.diffuseColor = defaultDiffuse;
			materialProperties.hasSpecular = false;
			// Check for duplicate material
			vector<Material>::iterator it = find(materials.begin(), materials.end(), materialProperties);
			if (it != materials.end()) // Duplicate exists, copy material ids from duplicate
			{
				int index = distance(materials.begin(), it);
				for (unsigned int i = 0; i < currentMaterialIndices.size(); i++)
					materialIndices.push_back(index);
			}
			else // No duplicate found, push back new material and add new indices
			{
				materials.push_back(materialProperties);
				for (unsigned int i = 0; i < currentMaterialIndices.size(); i++)
					materialIndices.push_back(materialNumber);
				materialNumber++;
			}
			//AddMaterial(materialProperties, materials, materialIndices);
		}
		else // Node contains one or more materials
		{
			int currentValidIndex = 0;
			int startMaterialNumber = materialNumber;
			vector<int> materialOffsets(currentMaterialIndices.size(), 0);
			for (int materialIndex = 0; materialIndex < node->GetMaterialCount(); materialIndex++)
			{
				FbxSurfaceMaterial *material = node->GetMaterial(materialIndex);
				if (!material)
				{
					cout << "Texture import error: Could not get material from mesh node" << endl << endl;
					return;
				}

				// Ignore material if it is not in the material index array
				if (find(currentMaterialIndices.begin(), currentMaterialIndices.end(), materialIndex) == currentMaterialIndices.end())
					continue;

				// Get material info
				Material materialProperties;

				// Get diffuse info
				materialProperties.diffuseMap = GetTexturePath(material, FbxSurfaceMaterial::sDiffuse);
				materialProperties.diffuseColor = ((FbxSurfacePhong*)material)->Diffuse;

				// Get opacity info
				FbxDouble opacityFactor = ((FbxSurfacePhong*)material)->TransparencyFactor;
				materialProperties.hasOpacity = (opacityFactor > 0);
				if (materialProperties.hasOpacity)
					materialProperties.opacity = opacityFactor;

				// Get bump info
				materialProperties.normalMap = GetTexturePath(material, FbxSurfaceMaterial::sNormalMap);

				// Get specular info
				FbxDouble specularFactor = ((FbxSurfacePhong*)material)->SpecularFactor;
				materialProperties.hasSpecular = (specularFactor > 0);
				if (materialProperties.hasSpecular)
				{
					materialProperties.specularMap = GetTexturePath(material, FbxSurfaceMaterial::sSpecular);
					materialProperties.specularColor = ((FbxSurfacePhong*)material)->Specular;
					materialProperties.specularFactor = specularFactor;
				}

				// Get uv info
				FbxProperty property = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
				FbxTexture *texture = property.GetSrcObject<FbxTexture>();
				if (texture)
				{
					materialProperties.uvScaling[0] = texture->GetScaleU();
					materialProperties.uvScaling[1] = texture->GetScaleV();

					materialProperties.uvTranslation[0] = texture->GetTranslationU();
					materialProperties.uvTranslation[1] = texture->GetTranslationV();

					materialProperties.uvRotation[0] = texture->GetRotationU();
					materialProperties.uvRotation[1] = texture->GetRotationV();
					materialProperties.uvRotation[2] = texture->GetRotationW();
				}

				// Check if this is a duplicate material
				vector<Material>::iterator it = find(materials.begin(), materials.end(), materialProperties);
				if (it != materials.end()) // Duplicate found, set material indices to duplicate indices
				{
					//cout << "Duplicate material found" << endl;
					int index = distance(materials.begin(), it);
					replace(currentMaterialIndices.begin(), currentMaterialIndices.end(), materialIndex, index);
				}
				else // No duplicate exists
				{
					//cout << "No duplicate material found" << endl;
					materials.push_back(materialProperties);
					for (unsigned int i = 0; i < currentMaterialIndices.size(); i++) // Set material offsets
					{
						if (currentMaterialIndices[i] == materialIndex)
							materialOffsets[i] = startMaterialNumber;
					}
					if (currentValidIndex != materialIndex) // Normalize material array to only include valid Ids
						replace(currentMaterialIndices.begin(), currentMaterialIndices.end(), materialIndex, currentValidIndex);
					currentValidIndex++;
					materialNumber++;
				}
				//AddMaterial(materialProperties, materials, materialIndices);
			}
			for (unsigned int i = 0; i < currentMaterialIndices.size(); i++)
				materialIndices.push_back(currentMaterialIndices[i] + materialOffsets[i]);
		}
		//for (int i = 0; i < materialIndices.size(); i++)
		//	cout << materialIndices[i];
		//cout << endl;
	}
}

void MeshImporter::GetMaterialIndices(FbxMesh *mesh, vector<int> &materialIndices)
{
	FbxGeometryElementMaterial *material = mesh->GetElementMaterial(0);
	//cout << "Num Materials: " << material->GetIndexArray().GetCount() << endl;
	if (!material || material->GetMappingMode() == FbxLayerElement::EMappingMode::eAllSame)
	{
		for (int i = 0; i < mesh->GetPolygonCount(); i++)
		{
			materialIndices.push_back(0);
			//cout << materialIndices[startMaterialIndex] << " ";
		}
		//cout << endl << endl;
	}
	else if (material->GetMappingMode() == FbxLayerElement::EMappingMode::eByPolygon)
	{
		for (int i = 0; i < material->GetIndexArray().GetCount(); i++)
		{
			materialIndices.push_back(material->GetIndexArray().GetAt(i));
			//cout << materialIndices[material->GetIndexArray().GetAt(i) + startMaterialIndex] << " ";
		}
		//cout << endl << endl;
	}
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