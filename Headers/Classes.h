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

// Utilities
class Transformation;
class Light;
struct Material;

// Types
struct BezierCurveParams;
struct BezierSurfaceParams;
struct BSplineParams;
struct DiscreteCurveParams;
struct MeshRenderParams;
struct MeshRenderSelectionParams;
struct ModelBaseParams;
struct ModelLoaderReturn;
struct ModelParams;
struct RenderParams;
struct RenderShadowParams;
struct SUpdateInfo;

// Interfaces
interface IDrawable;
interface IPrintable;

// Wrapper
class CMyApp;