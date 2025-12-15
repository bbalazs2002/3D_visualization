#pragma once

#include "include_all.h"

class ModelBase : public IDrawable, public IPrintable {
protected:
	inline static int count = 0;
	GLuint m_programID;
	GLuint m_programSelectedID;
	std::vector<Transformation*> m_transforms{};
	glm::mat4 m_transform{glm::identity<glm::mat4>()};
	bool m_show;
	int m_drawMode;
	std::string m_name;
	bool m_transformDirty = true;
	bool m_applyTransforms = true;
	MODEL_TYPE_TYPE m_type;
	bool m_deleteMarker = false;

	// ImGui buffers
	static inline float m_rotationAngleX = 0;
	static inline float m_rotationAngleY = 0;
	static inline float m_rotationAngleZ = 0;
	static inline float m_translateX = 0;
	static inline float m_translateY = 0;
	static inline float m_translateZ = 0;
	static inline float m_scaleX = 1;
	static inline float m_scaleY = 1;
	static inline float m_scaleZ = 1;
	static inline float m_shearX = 0;
	static inline float m_shearY = 0;
	static inline float m_shearZ = 0;
	static inline float m_bezierCutParam = 0;
	static inline float m_newKnot = 0;

	static inline glm::vec3 m_curveColor{ 1, 0, 1 };
	static inline glm::vec3 m_bezierNewCtrlPoint{ 0, 0, 0 };
	static inline glm::vec3 m_bsplineNewCtrlPoint{ 0, 0, 0 };
	static inline glm::vec3 m_discreteCurveNewCtrlPoint{ 0, 0, 0 };

	char m_objNameBuffer[64] = "";
	
public:
	ModelBase(ModelBaseParams params) {
		m_type = MODEL_TYPE_MODEL;
		m_programID = params.programID;
		m_programSelectedID = params.programSelectedID;
		m_show = params.show;
		m_drawMode = params.drawMode;
		AddTransform();
		if (std::strcmp("", params.name) == 0) {
			m_name = "Model " + std::to_string(ModelBase::count);
		}
		else {
			m_name = params.name;
		}
		strcpy_s(m_objNameBuffer, m_name.c_str());

		++ModelBase::count;
	}

	// IDrawable methods
	virtual void Render(RenderParams* p) override = 0;
	virtual void RenderSelection(RenderParams* p) override = 0;
	virtual void RenderGUI(std::vector<ModelBase*>* models) = 0;
	void RenderGUIBase() override;

	void MarkForDeletion() {
		m_deleteMarker = true;
	}
	bool MarkedForDeletion() {
		return m_deleteMarker;
	}

	std::string GetName() const {
		return m_name;
	}
	void SetName() {
		if (std::strcmp("", m_objNameBuffer) == 0) {
			Log::errorToConsole("Invalid object name");
			return;
		}
		m_name = m_objNameBuffer;
	}
	void SetName(const char* name) {
		strcpy_s(m_objNameBuffer, name);
		SetName();
	}

	GLuint GetProgramID() const {
		return m_programID;
	}
	void SetProgramID(GLuint programID) {
		m_programID = programID;
	}

	GLuint GetProgramSelectedID() const {
		return m_programSelectedID;
	}
	void SetProgramSelectedID(GLuint programID) {
		m_programSelectedID = programID;
	}

	Transformation GetTransform(int index) const {
		if (index >= m_transforms.size()) {
			Log::errorToConsole("Model::GetTransform index out of range %d", index);
			return Transformation();
		}
		return *m_transforms[index];
	}
	virtual glm::mat4 GetTransform() {
		// check if any of the transformations is changed
		bool isDirty = false;
		for (auto t : m_transforms) {
			if (t->IsDirty()) {
				isDirty = true;
				t->Clean();
			}
		}
		// calculate transformation if changed
		if (isDirty || m_transformDirty) {
			m_transformDirty = false;
			glm::mat4 acc = glm::identity<glm::mat4>();
			for (int i = m_transforms.size() - 1; i >= 0; --i) {
				acc *= m_transforms[i]->Get();
			}
			m_transform = acc;
		}
		return m_transform;
	}
	std::vector<Transformation*> GetTransforms() const {
		return m_transforms;
	}
	void SetTransforms(std::vector<Transformation*> transforms) {
		m_transforms.clear();
		for (Transformation* t : transforms) {
			m_transforms.push_back(new Transformation(t->Get()));
		}
		m_transformDirty = true;
	}
	void AddTransform(glm::mat4 transform) {
		m_transforms.push_back(new Transformation(transform));
	}
	void AddTransform() {
		m_transforms.push_back(new Transformation());
	}
	void DelTransform(int index) {
		if (index >= 0 && index < m_transforms.size()) {
			delete(m_transforms[index]);
			m_transforms.erase(m_transforms.begin() + index);
			m_transformDirty = true;
		}
		if (m_transforms.size() < 1) {
			AddTransform();
		}
	}
	void DelTransforms() {
		for (auto t : m_transforms) {
			delete(t);
		}
	}

	void SetShow(bool show) {
		m_show = show;
	}
	bool GetShow() const {
		return m_show;
	}

	void SetDrawMode(int drawMode) {
		m_drawMode = drawMode;
	}
	int GetDrawMode() const {
		return m_drawMode;
	}

	virtual void SetApplyTransforms(bool apply) {
		m_applyTransforms = apply;
	}
	bool GetApplyTransforms() const {
		return m_applyTransforms;
	}

	MODEL_TYPE_TYPE GetType() const {
		return m_type;
	}

	// iterator
	std::vector<Transformation*>::iterator begin() {
		return m_transforms.begin();
	}
	std::vector<Transformation*>::iterator end() {
		return m_transforms.end();
	}
	std::vector<Transformation*>::const_iterator begin() const {
		return m_transforms.begin();
	}
	std::vector<Transformation*>::const_iterator end() const {
		return m_transforms.end();
	}

	std::string toString() {
		std::stringstream str;
		str << "Name: " << m_name << std::endl;
		str << "Show: " << (m_show ? "true" : "false") << std::endl;
		// str << "Wireframe: " << (m_wireFrame ? "true" : "false") << std::endl;
		str << "Program ID: " << m_programID << std::endl;
		// str << "Texture ID: " << m_textureID << std::endl;
		str << "Draw mode: " << m_drawMode << std::endl;
		// str << "File path: " << m_objPath << std::endl;
		str << "Transformations: " << std::endl;

		int count = 0;
		for (auto t : m_transforms) {
			str << "  Transformation #" << count++ << std::endl;
			glm::mat4 temp = glm::transpose(t->Get());
			str << "   " << temp[0][0] << ", " << temp[0][1] << ", " << temp[0][2] << ", " << temp[0][3] << std::endl;
			str << "   " << temp[1][0] << ", " << temp[1][1] << ", " << temp[1][2] << ", " << temp[1][3] << std::endl;
			str << "   " << temp[2][0] << ", " << temp[2][1] << ", " << temp[2][2] << ", " << temp[2][3] << std::endl;
			str << "   " << temp[3][0] << ", " << temp[3][1] << ", " << temp[3][2] << ", " << temp[3][3] << std::endl;
		}
		str << "Final transformation: " << std::endl;
		glm::mat4 temp = glm::transpose(GetTransform());
		str << "   " << temp[0][0] << ", " << temp[0][1] << ", " << temp[0][2] << ", " << temp[0][3] << std::endl;
		str << "   " << temp[1][0] << ", " << temp[1][1] << ", " << temp[1][2] << ", " << temp[1][3] << std::endl;
		str << "   " << temp[2][0] << ", " << temp[2][1] << ", " << temp[2][2] << ", " << temp[2][3] << std::endl;
		str << "   " << temp[3][0] << ", " << temp[3][1] << ", " << temp[3][2] << ", " << temp[3][3] << std::endl;

		return str.str();
	}

	~ModelBase() {
		DelTransforms();
	}

};