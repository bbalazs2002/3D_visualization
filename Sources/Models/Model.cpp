#include "../../Headers/include_all.h"

Model::Model(ModelParams params) : ModelBase(MODEL2MODELBASE) {
	m_wireframe = params.wireFrame;
}
Model::~Model() {
	CleanGeometry();
}

void Model::Render(RenderParams* p) {
	if (!GetShow()) {
		return;
	}

	// Log::logToConsole("Render model: ", GetName());

	MeshRenderParams mp {
		p->lineWidth,
		p->cameraPos,
		p->lights,
		p->lightCount,
		p->modelIndex,
		p->cursorPos,
		p->windowSize,
		p->viewProj,
		GetProgramID(),
		GetWireFrame(),
		GetApplyTransforms(),
		GetTransform(),
		GetDrawMode()
	};
	MeshRenderSelectionParams msp {
		p->cameraPos,
		p->viewProj,
		p->selectionWidth,
		p->selectionColor,
		GetProgramSelectedID(),
		GetApplyTransforms(),
		GetTransform(),
		GetDrawMode()
	};

	if (
		!p->selected ||
		(p->selected && (CMyApp::MeshID < 0 || CMyApp::MeshID >= m_meshes.size()))
	) {
		// Render all meshes
		for (Mesh* mesh : m_meshes) {
			mesh->Render(&mp);
			if (p->selected) {
				mesh->RenderSelection(&msp);
			}
		}
	}
	else {
		// render only one mesh
		Mesh* mesh = m_meshes[CMyApp::MeshID];
		mesh->Render(&mp);
		if (p->selected && p->selectionWidth > 0) {
			mesh->RenderSelection(&msp);
		}
	}
	
}
void Model::RenderSelection(RenderParams* p) {
	return;
}
void Model::RenderGUI(std::vector<ModelBase*>*) {
	ImGui::Text("Model specific options");
	ImGui::Spacing();

	Model* m = this;

	// Wireframe
	bool wireframe = m->GetWireFrame();
	if (ImGui::Checkbox("Wireframe", &wireframe)) {
		m->SetWireFrame(wireframe);
	}

	// OBJ file
	ImGui::InputText("Obj file path", m->m_objPathBuffer, IM_ARRAYSIZE(m->m_objPathBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		m->SetObjPath();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}

void Model::SetObjPath() {
	if (!std::filesystem::exists(m_objPathBuffer)) {
		return;
	}
	std::string tempPath = m_objPathBuffer;
	if (tempPath == m_objPath) {
		return;
	}
	m_objPath = tempPath;

	// clean old geometry
	CleanGeometry();
	// load the new file
	ModelLoaderReturn meshMatData = ModelLoader::LoadFromOBJ(m_objPath);
	m_meshes = meshMatData.meshes;
	m_materials = meshMatData.materials;
}