#include "LyrePch.h"
#include "Mesh.h"
#include "Renderer.h"
#include "PipelineResources/Shader.h"

Lyre::CMesh::CMesh(std::string const& filename)
	: m_model(1.f)
{
	std::ifstream fin(filename);

	struct SVertex
	{
		std::array<float, 3> position;
		std::array<float, 3> normal;
	};

	std::vector<SVertex> vertexData;
	std::vector<std::array<float,3>> normals;
	std::vector<unsigned> indexData;

	static unsigned const MaxLineSize = 100;
	for (char buffer[MaxLineSize]; fin.getline(buffer, MaxLineSize);)
	{
		std::stringstream ss{ buffer };
		SVertex vertex;
		std::string s;

		ss >> s;
		if (s == "v")
		{
			ss >> vertex.position[0] >> vertex.position[1] >> vertex.position[2];
			vertexData.push_back(vertex);
		}
		else if (s == "vn")
		{
			ss >> vertex.normal[0] >> vertex.normal[1] >> vertex.normal[2];
			normals.push_back(vertex.normal);
		}
		else if (s == "f")
		{
			// Faces are stored in the following format:
			// f v0/vt0/vn0 v1/vt1/vn1 v2/vt2/vn2 ...
			std::vector<unsigned> indices;
			indices.reserve(4);
			while (!ss.eof())
			{
				unsigned vertexIndex, textureIndex, normalIndex;
				ss >> vertexIndex;
				ss.seekg(1, std::ios_base::cur);
				ss >> textureIndex;
				ss.seekg(1, std::ios_base::cur);
				ss >> normalIndex;

				--vertexIndex;
				--textureIndex;
				--normalIndex;

				vertexData[vertexIndex].normal[0] = normals[normalIndex][0];
				vertexData[vertexIndex].normal[1] = normals[normalIndex][1];
				vertexData[vertexIndex].normal[2] = normals[normalIndex][2];

				indices.push_back(vertexIndex);
			}

			if (indices.size() == 3)
			{
				indexData.insert(indexData.end(), indices.begin(), indices.end());
			}
			else if (indices.size() == 4)
			{
				indexData.insert(indexData.end(), {
					indices[0], indices[1], indices[2],
					indices[0], indices[2], indices[3]
				});
			}
		}
	}

	unsigned vertexSize = sizeof(SVertex) / sizeof(float);
	m_vertexBuffer = CRenderer::GetAPI()->CreateVertexBuffer((float*)vertexData.data(), vertexData.size() * vertexSize);
	m_indexBuffer = CRenderer::GetAPI()->CreateIndexBuffer(indexData.data(), indexData.size());

	m_vertexBuffer->SetLayout(CRenderer::GetAPI()->CreateInputLayout({
		{ EShaderDataType::Float3, "POSITION" },
		{ EShaderDataType::Float3, "NORMAL" }
	}));
}

void Lyre::CMesh::SetShader(std::shared_ptr<CShader> shader)
{
	m_shader = shader;

	std::shared_ptr<CConstantBuffer> constantBuffer = CRenderer::GetAPI()->CreateConstantBuffer({
		EShaderDataType::Matrix
	});
	constantBuffer->UpdateConstant(0, glm::value_ptr(m_model));

	m_shader->AddConstantBuffer(constantBuffer);
}
