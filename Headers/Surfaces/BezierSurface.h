#pragma once

#include "../include_all.h"

class BezierSurface : public ModelBase {
protected:
	std::vector<glm::vec4> m_interpolatedPoints{};
	Material* m_material{};
	std::vector<glm::vec4> m_ctrlPoints{};
	glm::ivec2 m_dim{ 0, 0 };
	GLuint m_ctrlPointsSSBOID = 0;
	bool m_ctrlPointsDirty = false;
	GLuint m_interpolatedPointsSSBOID = 0;
	glm::ivec2 m_smoothness{10, 10};
	bool m_wireframe = false;

	void SetCtrlPointsSSBO() {
		glGenBuffers(1, &m_ctrlPointsSSBOID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ctrlPointsSSBOID);
		WriteCtrlPointsSSBO();
	}
	void WriteCtrlPointsSSBO() {
		// apply world transformation on 
		std::vector<glm::vec4> newPoints;
		for (int i = 0; i < m_ctrlPoints.size(); ++i) {
			if (m_applyTransforms) {
				newPoints.push_back(GetTransform() * m_ctrlPoints[i]);
			}
			else {
				newPoints.push_back(m_ctrlPoints[i]);
			}
		}
		// write to buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ctrlPointsSSBOID);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			newPoints.size() * sizeof(glm::vec4),
			newPoints.data(),
			GL_STATIC_DRAW);
	}
	void SetInterpolatedPointsSSBO() {
		glGenBuffers(1, &m_interpolatedPointsSSBOID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_interpolatedPointsSSBOID);
	}
	void WriteInterpolatedPointsSSBO() {
		if (GetInterpolatedPointsCount() <= 0) {
			return;
		}
		std::vector<glm::vec4> newPoints;
		for (auto& p : m_interpolatedPoints) {
			if (m_applyTransforms)
				newPoints.push_back(GetTransform() * p);
			else
				newPoints.push_back(p);
		}
		m_interpolatedPoints = newPoints;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_interpolatedPointsSSBOID);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			m_interpolatedPoints.size() * sizeof(glm::vec4),
			m_interpolatedPoints.data(),
			GL_STATIC_DRAW);
	}
public:
	BezierSurface(BezierSurfaceParams params);
	~BezierSurface();

	void Render(RenderParams* p) override;
	void RenderSelection(RenderParams* p) override;
	void RenderGUI(std::vector<ModelBase*>* models) override;

	void RenderInterpolatedPoints(RenderParams* p);

	inline void SetWireFrame(bool wireframe) {
		m_wireframe = wireframe;
	}
	inline bool GetWireFrame() {
		return m_wireframe;
	}

	inline void SetSmoothness(glm::vec2 smoothness) {
		m_smoothness = smoothness;
	}
	inline glm::ivec2 GetSmoothness() const {
		return m_smoothness;
	}

	inline void SetMaterial(Material* material) {
		if (m_material != nullptr) {
			delete(m_material);
		}
		m_material = material;
	}
	inline Material* GetMaterial() const {
		return m_material;
	}

	// prevent GetTransform to reset m_transformDirty and Transformation::m_dirty
	inline glm::mat4 GetTransform() const {
		return m_transform;
	}

	inline void AddCtrlRow(std::vector<glm::vec4> points) {
		if (points.size() != m_dim.y) {
			Log::errorToConsole("BezierSurface::AddCtrlRow columns count do not match");
			return;
		}
		m_ctrlPoints.insert(m_ctrlPoints.end(), points.begin(), points.end());
		++m_dim.x;
		m_ctrlPointsDirty = true;
	}
	inline void AddCtrlCol(std::vector<glm::vec4> points) {
		if (points.size() != m_dim.x) {
			throw std::invalid_argument("BezierSurface::AddCtrlCol rows count do not match");
		}

		int old_cols = m_dim.y;
		++m_dim.y;

		// create new ctrlPoints vector
		std::vector<glm::vec4> new_data;
		new_data.reserve(m_dim.x*m_dim.y);

		// insert ctrlPoints
		for (int r = 0; r < m_dim.x; ++r) {
			// old ctrl points
			for (size_t c = 0; c < old_cols; ++c) {
				new_data.push_back(m_ctrlPoints[r * old_cols + c]);
			}
			// new ctrl point
			new_data.push_back(points[r]);
		}

		m_ctrlPoints = std::move(new_data);
	}
	inline void DelCtrlRow(int index) {
			if (m_dim.x <= 0) {
				Log::errorToConsole("BezierSurface::DelCtrlRow unable to delete (tere are ", m_dim.x, " rows)");
				return;
			}
			if (index < 0 || index > m_dim.x - 1) {
				Log::errorToConsole("BezierSurface::DelCtrlRow invalid row index");
				return;
			}

			size_t start_index = index * m_dim.y;
			auto start_it = m_ctrlPoints.begin() + start_index;
			auto end_it = m_ctrlPoints.begin() + start_index + m_dim.y;
			m_ctrlPoints.erase(start_it, end_it);

			m_ctrlPointsDirty = true;
			--m_dim.x;
	}
	inline void DelCtrlCol(int index) {
		if (m_dim.y <= 0) {
			Log::errorToConsole("BezierSurface::DelCtrlCol unable to delete (tere are ", m_dim.y, " cols)");
			return;
		}
		if (index < 0 || index > m_dim.y - 1) {
			Log::errorToConsole("BezierSurface::DelCtrlCol invalid col index");
			return;
		}

		size_t new_cols = m_dim.y - 1;
		std::vector<glm::vec4> new_data;
		new_data.reserve(m_dim.x * new_cols);

		for (size_t r = 0; r < m_dim.x; ++r) {
			for (size_t c = 0; c < m_dim.y; ++c) {
				if (c != index) {
					new_data.push_back(m_ctrlPoints[r * m_dim.y + c]);
				}
			}
		}

		m_ctrlPointsDirty = true;
		m_ctrlPoints = std::move(new_data);
		m_dim.y = new_cols;
	}
	
	inline void SetCtrlPoint(glm::vec2 index, glm::vec3 position) {
		int ind = m_dim.y * index.x + index.y;
		if (ind < 0 || ind > m_ctrlPoints.size() - 1) {
			Log::errorToConsole("BezierSurface::SetCtrlPoint invalid index");
			return;
		}
		m_ctrlPoints[ind] = glm::vec4(position, 1);
		m_ctrlPointsDirty = true;
	}
	inline void SetCtrlPoints(glm::vec2 dim, std::vector<glm::vec4> points) {
		if (points.size() != dim.x * dim.y) {
			Log::errorToConsole("BezierSurface::SetCtrlPoints points count and dimensions do not match");
			return;
		}
		m_dim = dim;
		m_ctrlPoints = points;
		m_ctrlPointsDirty = true;
	}
	inline void SetCtrlPoints(glm::vec2 dim, std::vector<glm::vec3> points) {
		if (points.size() != dim.x * dim.y) {
			Log::errorToConsole("BezierSurface::SetCtrlPoints points count and dimensions do not match");
			return;
		}
		m_ctrlPoints.clear();
		for (auto p : points) {
			m_ctrlPoints.push_back(glm::vec4(p, 1));
		}
		m_dim = dim;
		m_ctrlPointsDirty = true;
	}
	inline void SetCtrlPoints(std::vector<std::vector<glm::vec3>> grid) {
		// validate grid
		m_dim = glm::vec2(0, 0);
		m_ctrlPoints.clear();
		if (grid.size() <= 0) {
			return;
		}
		size_t cols = grid[0].size();
		for (auto g : grid) {
			if (g.size() != cols) {
				Log::errorToConsole("BezierSurface::SetCtrlPoints invalid grid (uneven columns count)");
				return;
			}
		}


		// set ctrl points
		for (auto row : grid) {
			for (auto p : row) {
				m_ctrlPoints.push_back(glm::vec4(p, 1));
			}
			++m_dim.x;
		}
		m_dim.y = cols;
	}
	inline void SetCtrlPoints(std::vector<std::vector<glm::vec4>> grid) {
		// validate grid
		m_dim = glm::vec2(0, 0);
		m_ctrlPoints.clear();
		if (grid.size() <= 0) {
			return;
		}
		size_t cols = grid[0].size();
		for (auto g : grid) {
			if (g.size() != cols) {
				Log::errorToConsole("BezierSurface::SetCtrlPoints invalid grid (uneven columns count)");
				return;
			}
		}


		// set ctrl points
		for (auto row : grid) {
			for (auto p : row) {
				m_ctrlPoints.push_back(p);
			}
			++m_dim.x;
		}
		m_dim.y = cols;
	}

	inline std::vector<glm::vec4> GetCtrlPoints() const {
		return m_ctrlPoints;
	}
	inline GLuint GetCtrlPointsSSBO() const {
		return m_ctrlPointsSSBOID;
	}

	// Interpolated points
	inline void SetInterpolatedPoints(std::vector<glm::vec4> points) {
		if (m_interpolatedPoints.size() > 0) {
			Log::errorToConsole("Cannot overwrite interpolated points");
			return;
		}
		m_interpolatedPoints = points;
		SetInterpolatedPointsSSBO();
		WriteInterpolatedPointsSSBO();
	}
	inline void SetInterpolatedPoints(std::vector<glm::vec3> points) {
		if (m_interpolatedPoints.size() > 0) {
			Log::errorToConsole("Cannot overwrite interpolated points");
			return;
		}
		for (auto p : points) {
			m_interpolatedPoints.push_back(glm::vec4(p, 1));
		}
		SetInterpolatedPointsSSBO();
		WriteInterpolatedPointsSSBO();
	}
	inline void SetInterpolatedPoints(std::vector<std::vector<glm::vec4>> grid) {
		if (m_interpolatedPoints.size() > 0) {
			Log::errorToConsole("Cannot overwrite interpolated points");
			return;
		}
		for (auto row : grid) {
			for (auto p : row) {
				m_interpolatedPoints.push_back(p);
			}
		}
		SetInterpolatedPointsSSBO();
		WriteInterpolatedPointsSSBO();
	}
	inline void SetInterpolatedPoints(std::vector<std::vector<glm::vec3>> grid) {
		if (m_interpolatedPoints.size() > 0) {
			Log::errorToConsole("Cannot overwrite interpolated points");
			return;
		}
		for (auto row : grid) {
			for (auto p : row) {
				m_interpolatedPoints.push_back(glm::vec4(p, 1));
			}
		}
		SetInterpolatedPointsSSBO();
		WriteInterpolatedPointsSSBO();
	}
	inline void ClearInterpolatedPoints() {
		m_interpolatedPoints.erase(m_interpolatedPoints.begin(), m_interpolatedPoints.end());
		glDeleteBuffers(1, &m_interpolatedPointsSSBOID);
		m_interpolatedPointsSSBOID = 0;
	}
	inline std::vector<glm::vec4> GetInterpolatedPoints() {
		return m_interpolatedPoints;
	}
	inline GLuint GetInterpolatedPointsSSBO() const {
		return m_interpolatedPointsSSBOID;
	}
	inline int GetInterpolatedPointsCount() const {
		return m_interpolatedPoints.size();
	}

	inline void SetApplyTransforms(bool apply) {
		m_applyTransforms = apply;
		m_transformDirty = true;
	}

	inline glm::ivec2 GetDimensions() const {
		return m_dim;
	}
	inline int GetRowsCount() const {
		return m_dim.x;
	}
	inline int GetColsCount() const {
		return m_dim.y;
	}

	inline std::string toString() const {
		std::stringstream ss;
		for (int i = 0; i < m_dim.x; ++i) {
			for (int j = 0; j < m_dim.y; ++j) {
				ss << m_ctrlPoints[i * m_dim.y + j].x << "; ";
			}
			ss << std::endl;
		}
		return ss.str();
	}
};