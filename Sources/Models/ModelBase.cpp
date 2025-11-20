#include "../../Headers/include_all.h"

void ModelBase::RenderGUIBase() {
	// Object EDITOR WINDOW
	ImGui::Text("Object options");

	ModelBase* m = this;

	ImGui::InputText("name", m->m_objNameBuffer, IM_ARRAYSIZE(m->m_objNameBuffer));
	ImGui::SameLine();
	if (ImGui::Button("Rename")) {
		m->SetName();
	}

	// Visibility
	bool show = m->GetShow();
	if (ImGui::Checkbox("show", &show)) {
		m->SetShow(show);
	}

	// Transformations
	bool applyTransforms = m->GetApplyTransforms();
	if (ImGui::Checkbox("apply transforms", &applyTransforms)) {
		if (m->GetType() == MODEL_TYPE_BEZIER) {
			((Bezier*)m)->SetApplyTransforms(applyTransforms);
		}
		else {
			m->SetApplyTransforms(applyTransforms);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Transformations:");

	std::stringstream str;
	int TCount = 0;
	for (auto t : *m) {
		str.str(std::string());
		str << "transformation #" << TCount;
		if (ImGui::CollapsingHeader(str.str().c_str())) {

			glm::mat4 temp = t->Get();
			glm::mat4 arr = glm::transpose(temp);

			bool changed = false;
			for (int i = 0; i < 4; i++) {
				if (ImGui::InputFloat4(("Row " + std::to_string(TCount) + " " + std::to_string(i)).c_str(), &arr[i][0])) {
					changed = true;
				}
			}
			if (changed) {
				t->Set(glm::transpose(arr));
			}

			if (ImGui::Button(("Delete #" + std::to_string(TCount)).c_str())) {
				m->DelTransform(TCount);
				break;
			}

		}
		++TCount;
	}
	if (ImGui::CollapsingHeader("Collapsed transformations")) {
		glm::mat4 arr = glm::transpose(m->GetTransform());
		if (ImGui::BeginTable("Collapsed_transformations", 4, ImGuiTableFlags_Borders))
		{
			for (int row = 0; row < 4; ++row)
			{
				ImGui::TableNextRow();
				for (int col = 0; col < 4; ++col)
				{
					ImGui::TableSetColumnIndex(col);
					ImGui::Text("%f", arr[row][col]);
				}
			}
			ImGui::EndTable();
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// New transformation
	if (ImGui::CollapsingHeader("Identity")) {
		if (ImGui::Button("Add")) {
			m->AddTransform();
		}
	}

	if (ImGui::CollapsingHeader("Translate")) {
		ImGui::SliderFloat("X", &m_translateX, -100.f, 100.f);
		ImGui::SliderFloat("Y", &m_translateY, -100.f, 100.f);
		ImGui::SliderFloat("Z", &m_translateZ, -100.f, 100.f);
		if (ImGui::Button("Add")) {
			m->AddTransform(glm::transpose(glm::mat4{
				{1, 0, 0, m_translateX},
				{0, 1, 0, m_translateY},
				{0, 0, 1, m_translateZ},
				{0, 0, 0, 1}
				}));
		}
	}

	if (ImGui::CollapsingHeader("Rotate")) {
		ImGui::SliderAngle("Angle X", &m_rotationAngleX);
		ImGui::SliderAngle("Angle Y", &m_rotationAngleY);
		ImGui::SliderAngle("Angle Z", &m_rotationAngleZ);
		if (ImGui::Button("Add")) {
			float cosX = glm::cos(m_rotationAngleX);
			float sinX = glm::sin(m_rotationAngleX);
			float cosY = glm::cos(m_rotationAngleY);
			float sinY = glm::sin(m_rotationAngleY);
			float cosZ = glm::cos(m_rotationAngleZ);
			float sinZ = glm::sin(m_rotationAngleZ);
			m->AddTransform(glm::transpose(glm::mat4{
				{cosY * cosZ,                       -sinZ * cosY,                      sinY,         0},
				{sinX * sinY * cosZ + cosX * sinZ,  -sinZ * sinX * sinY + cosX * cosZ, -sinX * cosY, 0},
				{-sinY * cosX * cosZ + sinX * sinZ, cosX * sinY * sinZ + sinX * cosZ,  cosX * cosY,  0},
				{0,                                 0,                                 0,            1}
				}));
		}
	}

	if (ImGui::CollapsingHeader("Scale / Reflect")) {
		ImGui::SliderFloat("X", &m_scaleX, -100.f, 100.f);
		ImGui::SliderFloat("Y", &m_scaleY, -100.f, 100.f);
		ImGui::SliderFloat("Z", &m_scaleZ, -100.f, 100.f);
		if (ImGui::Button("Add")) {
			m->AddTransform(glm::transpose(glm::mat4{
				{m_scaleX, 0,        0,        0},
				{0,        m_scaleY, 0,        0},
				{0,        0,        m_scaleZ, 0},
				{0,        0,        0,        1}
				}));
		}
	}

	if (ImGui::CollapsingHeader("Shear")) {
		ImGui::SliderAngle("Angle X", &m_shearX);
		ImGui::SliderAngle("Angle Y", &m_shearY);
		ImGui::SliderAngle("Angle Z", &m_shearZ);
		if (ImGui::Button("Add")) {
			float x = glm::tan(m_shearX);
			float y = glm::tan(m_shearY);
			float z = glm::tan(m_shearZ);
			m->AddTransform(glm::transpose(glm::mat4{
				{1, x,             x * (y + 1.f),                 0},
				{y, x * y + 1.f,   x * y + y * (x * y + 1.f),     0},
				{z, z * (x + 1.f), z * (y * (x + 1.f) + x) + 1.f, 0},
				{0, 0,             0,                             1}
				}));
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Mark object for deletion
	if (ImGui::Button("Delete")) {
		m_deleteMarker = true;
	}
}