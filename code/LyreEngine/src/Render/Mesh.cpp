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

	while (fin)
	{
		SVertex vertex;
		std::string s;

		fin >> s;
		if (s == "v")
		{
			fin >> vertex.position[0] >> vertex.position[1] >> vertex.position[2];
			vertexData.push_back(vertex);
		}
		else if (s == "vn")
		{
			fin >> vertex.normal[0] >> vertex.normal[1] >> vertex.normal[2];
			normals.push_back(vertex.normal);
		}
		else if (s == "f")
		{
			// Faces are stored in the following format:
			// f a/b/c d/e/f g/h/i
			unsigned a, d, g; // vertices
			unsigned c, f, i; // normals
			unsigned ignore_i; // integers to ignore
			char ignore_c; // characters to ignore
			fin >> a >> ignore_c >> ignore_i >> ignore_c >> c
				>> d >> ignore_c >> ignore_i >> ignore_c >> f
				>> g >> ignore_c >> ignore_i >> ignore_c >> i;

			--a; --c;
			--d; --f;
			--g; --i;

			vertexData[a].normal[0] = normals[c][0];
			vertexData[a].normal[1] = normals[c][1];
			vertexData[a].normal[2] = normals[c][2];

			vertexData[d].normal[0] = normals[f][0];
			vertexData[d].normal[1] = normals[f][1];
			vertexData[d].normal[2] = normals[f][2];

			vertexData[g].normal[0] = normals[i][0];
			vertexData[g].normal[1] = normals[i][1];
			vertexData[g].normal[2] = normals[i][2];

			indexData.insert(indexData.end(), { a, d, g });
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
