#pragma once

#include "../include_all.h"

using namespace Eigen;

class BezierSurfaceInterpolation {
public:
	enum ParametrizationMethod {
		Uniform,
		ChordLength
	};

protected:

	typedef std::vector<std::vector<Vector3d>> PointGrid;

	static inline ParametrizationMethod parametrizationMethodU = ParametrizationMethod::ChordLength;
	static inline ParametrizationMethod parametrizationMethodV = ParametrizationMethod::ChordLength;

	//
	// --- SEGÉDFÜGGVÉNYEK ---
	//
	
	// Bernstein-polinom kiszámítása (n-ed fokú, i-edik polinom, u paraméternél)
	static double bernstein(int i, int n, double u) {
		if (u < 0.0) u = 0.0;
		if (u > 1.0) u = 1.0;

		double combo = 1.0;
		for (int j = 1; j <= i; ++j) {
			combo = combo * (double)(n - i + j) / (double)j;
		}
		return combo * std::pow(u, i) * std::pow(1.0 - u, n - i);
	}

	// Húrhossz alapú paraméterezés egy pontsorozatra
	static VectorXd getChordLengthParams(const std::vector<Vector3d>& points) {
		int N = points.size();
		VectorXd u = VectorXd::Zero(N);
		double totalLength = 0.0;
		for (int i = 1; i < N; ++i) {
			totalLength += (points[i] - points[i - 1]).norm();
		}
		if (totalLength > 1e-9) {
			double currentLength = 0.0;
			for (int i = 1; i < N; ++i) {
				currentLength += (points[i] - points[i - 1]).norm();
				u[i] = currentLength / totalLength;
			}
		}
		else {
			for (int i = 0; i < N; ++i) u[i] = (double)i / (N - 1);
		}
		u[N - 1] = 1.0;
		return u;
	}

	// Egyenletes eloszlású paraméterek generálása
	static VectorXd getUniformParams(int N) {
		VectorXd u = VectorXd::Zero(N);
		for (int i = 0; i < N; ++i) {
			u[i] = (double)i / (double)(N - 1);
		}
		return u;
	}

	// Rács érvényességének ellenõrzése
	static bool isValidGrid(const std::vector<std::vector<glm::vec3>>& grid) {
		// 1. Üres rács ellenõrzése
		if (grid.empty()) return false;

		// 2. Elsõ sor üres-e
		size_t expectedCols = grid[0].size();
		if (expectedCols == 0) return false;

		// 3. Minden sor hossza megegyezik-e
		for (size_t i = 1; i < grid.size(); ++i) {
			if (grid[i].size() != expectedCols) {
				return false;
			}
		}

		return true;
	}

	// GLM pontháló konvertálása Eigen ponthálóra
	static PointGrid convertGLMToEigen(const std::vector<std::vector<glm::vec3>>& glmGrid) {
		if (!isValidGrid(glmGrid)) {
			Log::errorToConsole("Invalid point grid for Bezier-surface interpolation");
		}

		int rows = glmGrid.size();
		int cols = glmGrid[0].size();

		PointGrid eigenGrid(rows, std::vector<Eigen::Vector3d>(cols));

		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				eigenGrid[i][j] = Eigen::Vector3d(
					static_cast<double>(glmGrid[i][j].x),
					static_cast<double>(glmGrid[i][j].y),
					static_cast<double>(glmGrid[i][j].z)
				);
			}
		}

		return eigenGrid;
	}

	// Eigen pontháló konvertálása GLM ponthálóra
	static std::vector<std::vector<glm::vec3>> convertEigenToGLM(const PointGrid& eigenGrid) {
		int rows = eigenGrid.size();
		int cols = eigenGrid[0].size();

		std::vector<std::vector<glm::vec3>> glmGrid(rows, std::vector<glm::vec3>(cols));

		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				glmGrid[i][j] = glm::vec3(
					static_cast<float>(eigenGrid[i][j].x()),
					static_cast<float>(eigenGrid[i][j].y()),
					static_cast<float>(eigenGrid[i][j].z())
				);
			}
		}

		return glmGrid;
	}

	//
	// --- FÕ ALGORITMUS ---
	//

	// Bézier-felület interpoláció
	static std::vector<std::vector<glm::vec3>> interpolateBezierSurface(const PointGrid& D) {
		int n_points = D.size();       // Sorok száma
		int m_points = D[0].size();    // Oszlopok száma
		int n = n_points - 1;          // U irányú fokszám
		int m = m_points - 1;          // V irányú fokszám

		// 1. Paraméterezés (Átlagolt húrhossz)
		VectorXd u_params = VectorXd::Zero(n_points);
		VectorXd v_params = VectorXd::Zero(m_points);

		if (parametrizationMethodU == ChordLength) {
			std::vector<Vector3d> column;
			for (int j = 0; j < m_points; ++j) {
				column.clear();
				for (int i = 0; i < n_points; ++i) column.push_back(D[i][j]);
				u_params += getChordLengthParams(column);
			}
			u_params /= m_points;
		}
		else {
			u_params = getUniformParams(n_points);
		}
		
		if (parametrizationMethodV == ChordLength) {
			for (int i = 0; i < n_points; ++i) {
				v_params += getChordLengthParams(D[i]);
			}
			v_params /= n_points;
		}
		else {
			v_params = getUniformParams(m_points);
		}

		// 2. Interpoláció U irányban (Oszloponként)
		// Létrehozunk egy ideiglenes Q hálót
		PointGrid Q(n_points, std::vector<Vector3d>(m_points));

		MatrixXd A_u(n_points, n_points);
		for (int k = 0; k < n_points; ++k) {
			for (int i = 0; i < n_points; ++i) {
				A_u(k, i) = bernstein(i, n, u_params[k]);
			}
		}
		auto LU_u = A_u.fullPivLu();

		for (int j = 0; j < m_points; ++j) {
			VectorXd dx(n_points), dy(n_points), dz(n_points);
			for (int i = 0; i < n_points; ++i) {
				dx[i] = D[i][j].x(); dy[i] = D[i][j].y(); dz[i] = D[i][j].z();
			}
			VectorXd qx = LU_u.solve(dx);
			VectorXd qy = LU_u.solve(dy);
			VectorXd qz = LU_u.solve(dz);
			for (int i = 0; i < n_points; ++i) Q[i][j] = Vector3d(qx[i], qy[i], qz[i]);
		}

		// 3. Interpoláció V irányban (Soronként a Q hálón)
		PointGrid P(n_points, std::vector<Vector3d>(m_points)); // Végsõ kontrollpontok

		MatrixXd A_v(m_points, m_points);
		for (int l = 0; l < m_points; ++l) {
			for (int j = 0; j < m_points; ++j) {
				A_v(l, j) = bernstein(j, m, v_params[l]);
			}
		}
		auto LU_v = A_v.fullPivLu();

		for (int i = 0; i < n_points; ++i) {
			VectorXd qx(m_points), qy(m_points), qz(m_points);
			for (int j = 0; j < m_points; ++j) {
				qx[j] = Q[i][j].x(); qy[j] = Q[i][j].y(); qz[j] = Q[i][j].z();
			}
			VectorXd px = LU_v.solve(qx);
			VectorXd py = LU_v.solve(qy);
			VectorXd pz = LU_v.solve(qz);
			for (int j = 0; j < m_points; ++j) P[i][j] = Vector3d(px[j], py[j], pz[j]);
		}

		return convertEigenToGLM(P);
	}

public:

	// Paraméterezési módszer beállítása U irányban
	static inline void setParametrizationMethodU(ParametrizationMethod method) {
		parametrizationMethodU = method;
	}

	// Paraméterezési módszer beállítása V irányban
	static inline void setParametrizationMethodV(ParametrizationMethod method) {
		parametrizationMethodV = method;
	}

	// Bézier-felület interpoláció
	static std::vector<std::vector<glm::vec3>> interpolateBezierSurface(const std::vector<std::vector<glm::vec3>>& glmGrid) {
		return interpolateBezierSurface(convertGLMToEigen(glmGrid));
	}

	// tesztesetek
	static std::vector<std::vector<glm::vec3>> getOvershootTestGrid() {
		int rows = 5, cols = 5;
		// Széleken sûrû, középen nagy ugrás: 0.0, 0.5, [SZÜNET], 4.5, 5.0
		std::vector<float> x_coords = { 0.0f, 0.5f, 2.5f, 4.5f, 5.0f };
		std::vector<float> y_coords = { 0.0f, 0.5f, 2.5f, 4.5f, 5.0f };

		std::vector<std::vector<glm::vec3>> grid(rows, std::vector<glm::vec3>(cols));
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				float x = x_coords[j];
				float y = y_coords[i];
				// Egy sima harang alakú domb
				float z = exp(-(pow(x - 2.5f, 2) + pow(y - 2.5f, 2)) / 5.0f) * 3.0f;
				grid[i][j] = glm::vec3(x, z, y);
			}
		}
		return grid;
	}
	static std::vector<std::vector<glm::vec3>> getLShapedDensityGrid() {
		int n = 6;
		std::vector<std::vector<glm::vec3>> grid(n, std::vector<glm::vec3>(n));
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				// Logaritmikus eloszlás, hogy az origó közelében sûrûbb legyen
				float x = pow(2.0f, (float)j) - 1.0f;
				float y = pow(2.0f, (float)i) - 1.0f;
				float z = sin(x * 0.2f) * y * 0.5f;
				grid[i][j] = glm::vec3(x, z, y);
			}
		}
		return grid;
	}
	static std::vector<std::vector<glm::vec3>> getStretchingTestGrid() {
		int rows = 4, cols = 8;
		std::vector<std::vector<glm::vec3>> grid(rows, std::vector<glm::vec3>(cols));
		for (int i = 0; i < rows; ++i) {
			float currentX = 0.0f;
			for (int j = 0; j < cols; ++j) {
				// Minden lépéssel egyre nagyobb a távolság (1, 2, 3, 4...)
				currentX += (float)j * 0.5f;
				float y = (float)i * 2.0f;
				float z = (i % 2 == 0) ? 0.5f : -0.5f; // Enyhe hullámzás
				grid[i][j] = glm::vec3(currentX, z, y);
			}
		}
		return grid;
	}
};