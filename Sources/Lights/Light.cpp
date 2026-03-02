#include "../../Headers/include_all.h"

void Light::RenderShadowMap(std::vector<ModelBase*>* models) {
	int lightID = 0;
	for (auto l : lights) {
		// render to shadow map
		if (l->GetCastShadow()) {
			PointLight* pl;
			if (Utilities::instanceof(l, pl)) {
				// Point light -> Cube map
				ShadowMapController::BindCubeFBO();
				for (GLuint faceID = 0; faceID < 6; ++faceID) {
					ShadowMapController::BindCubeFaceToFBO(pl->GetShadowLayer(), faceID);
					for (auto m : *(models)) {
						ICastShadow* model = nullptr;
						if (Utilities::instanceof(m, model)) {
							if (model->GetCastShadow()) {
								model->RenderShadowMap(lightID, faceID);
							}
						}
					}
				}
				ShadowMapController::UnbindFBO();
			}
			else {
				// Spot & Directional light -> 2D Texture
				ShadowMapController::Bind2DFBO(l->GetShadowLayer());
				for (auto m : *(models)) {
					ICastShadow* model = nullptr;
					if (Utilities::instanceof(m, model)) {
						if (model->GetCastShadow()) {
							model->RenderShadowMap(lightID);
						}
					}
				}
				ShadowMapController::UnbindFBO();
			}
		}
		++lightID;
	}
}