#pragma once

#include "include_all.h"

class ModelBase;

// Curves
class BezierCurve;
class BSpline;
class DiscreteCurve;

// Surfaces
class BezierDurface;

// Models
class Mesh;
class Model;
class ModelLoader;

// Lights
class Light;
class DirectionalLight;
class PointLight;
class SpotLight;

// Utilities
class Transformation;
struct Material;
class ShadowMapController;

// Types
struct BezierCurveParams;
struct BezierSurfaceParams;
struct BSplineParams;
struct DiscreteCurveParams;
struct MeshRenderParams;
struct MeshRenderSelectionParams;
struct MeshRenderShadowParams;
struct ModelBaseParams;
struct ModelLoaderReturn;
struct ModelParams;
struct RenderParams;
struct RenderShadowParams;
struct SUpdateInfo;

// Interfaces
interface IDrawable;
interface IPrintable;
interface ICastShadow;

// Wrapper
class CMyApp;