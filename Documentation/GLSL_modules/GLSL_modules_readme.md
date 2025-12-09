# Shader Modules Documentation

<img width="2967" height="1631" alt="Modules" src="https://github.com/user-attachments/assets/593d0b66-4d43-428c-bb0b-37722e0e9e50" />

## Shader Module Structure and Conventions

To ensure efficient inclusion and usage of shader modules, each module is organized into separate files. This design allows only the necessary components to be loaded when a module is used. All modules adhere to the following conventions:

TODO : Add content table and anchors

### 1. Module File Structure
Each module consists of **two files**:

- **`[module]_uniforms.glsl`** – Contains all uniforms, buffers, and preprocessor macros required for the module to function.  
  - If a uniform is a non-primitive type, its definition is included in this file.

- **`[module].glsl`** – Contains the module’s functions.  
  - If a function’s parameters or return type are non-primitive types, their definitions are included here.

### 2. Buffer Binding Conventions
If a module uses any buffers, the corresponding **binding points must be defined as preprocessor macros**. This ensures consistency and allows easy configuration when including the module in shaders.

---

## ClickHandler Module

The **ClickHandler** module enables models to be clickable. When included in a fragment shader, it writes the ID of the model currently under the cursor into a buffer.  

### Buffer Requirements
The module expects a buffer consisting of **two `vec4` elements**:

- The **first `vec4`** stores the corresponding model IDs for each element.  
- The **second `vec4`** contains unused data and should not be read, but it is **required for the module to function correctly**.

### Usage
Calling the `ClickHandler` function in the main shader function performs all necessary operations to track clickable models.

### Notes
- This module performs an **early depth test**.  
- It is therefore **not compatible** with techniques such as per-pixel displacement mapping.

### Structs
**ClickHandlerUniforms**
- windowSize : ivec2
- cursorPos : ivec2
- modelID : int

### Uniform Instances
- `clickHandlerData` : `ClickHandlerUniforms`

### Functions
- `ClickHandler() : void`

### Preprocessor Macros
- `CLICK_HANDLER_MODEL_SSBO`

---

## Transform Module

The **Transform** module is responsible for positioning the model within the scene.  

### Structs
**TransformUniforms**
- world : mat4

### Uniform Instances
- `transformData` : `TransformUniforms`

### Functions
- `Transform(pos : vec4) : vec4`

---

## BezierCurve Module

The **BezierCurve** module generates points along a Bézier curve based on the control points provided in an SSBO and a division value.  

### Functionality
- The module uses the **closed-form calculation** with the Bernstein basis.  
- The **basis function** can also be called independently.  
- This module depends on the **Math** module (`math.glsl`).

### Structs
**BezierCurveUniforms**
- ctrlPointCount : int
- division : int

**BezierParams**
- ctrlPointCount : int
- t : float

### Uniform Instances
- `bezierCurveData` : `BezierCurveUniforms`

### Functions
- `BernsteinBase(n : int, k : int, t : float) : float`
- `Bezier(params : BezierParams) : vec3`

### Preprocessor Macros
- `BEZIER_CURVE_CTRL_POINTS_SSBO`

---

## BSpline Module

The **BSpline** module generates points along a B-spline curve based on the control points and knot vector provided in an SSBO, as well as a division value.  

### Functionality
- Supports curves of up to **degree 10**.  
- This maximum degree can be increased by modifying a configuration in the source code.  

### Structs
**BSplineUniforms**
- degree : int
- knotCount : int
- ctrlPointCount : int
- division : int

**BSplineParams**
- degree : int
- t : float
- knotCount : int
- ctrlPointCount : int

**GetTParams**
- degree : int
- knotCount : int

### Uniform Instances
- `bSplineData` : `BSplineUniforms`

### Functions
- `BSplineGetTStart(params : GetTParams) : float`
- `BSplineGetTEnd(params : GetTParams) : float`
- `BSplineFindKnotSpan(params : BSplineParams) : int`
- `BSplineEvaluateBasisFunctions(spanIndex : int, params : BSplineParams, N : float*) : void`
- `BSpline(params : BSplineParams) : vec3`

### Preprocessor Macros
- `BSPLINE_CTRL_POINTS_SSBO`
- `BSPLINE_KNOTS_SSBO`

---

TODO : correct readme after this point and add comments

## BezierSurface Module

### Structs
**BezierSurfaceUniforms**
- ctrlPointCount : ivec2
- division : ivec2

### Uniform Instances
- `bezierSurfaceData` : `BezierSurfaceUniforms`

### Functions
- `BernsteinBaseDerivative(int n, int k, float t) : float`
- `BezierSurface_du(BezierSurfaceParams params) : vec3`
- `for(int i = 0; i <= n; ++i) : direction`
- `BezierSurface_dv(BezierSurfaceParams params) : vec3`
- `for(int i = 0; i <= n; ++i) : direction`
- `BezierSurface(BezierSurfaceParams params) : vec3`

### Preprocessor Macros
- `BEZIER_SURFACE_CTRL_POINTS_SSBO`
- `"BEZIER_SURFACE_CTRL_POINTS_SSBO`

---

## Camera Module

### Structs
**CameraUniforms**
- viewProj : mat4
- at : vec3
- up : vec3
- eye : vec3

### Uniform Instances
- `cameraData` : `CameraUniforms`

### Functions
- `CameraViewProj(vec4 pos) : vec4`

---

## Light Module

### Structs
**Light**
- La_const : vec4
- Ld_linear : vec4
- Ls_quadratic : vec4
- direction : vec4
- position : vec4
- type_angle : vec4

**LightUniforms**
- lightCount : int

### Uniform Instances
- `lightData` : `LightUniforms`

### Functions
- `LightCalculateContribution(LightCalculateContributionParams params) : vec3`
- `if(attenuation <= 0.0 || spotIntensity <= 0.0) : calculations`
- `LightCalculate(LightCalculateParams params) : vec3`

### Preprocessor Macros
- `LIGHT_LIGHTS_SSBO`
- `"LIGHT_LIGHTS_SSBO`
- `LIGHT_TYPE_DIRECTIONAL`
- `LIGHT_TYPE_DIRECTIONAL`
- `LIGHT_TYPE_POINT`
- `LIGHT_TYPE_POINT`
- `LIGHT_TYPE_SPOT`
- `LIGHT_TYPE_SPOT`

---

## Color Module

### Structs
**ColorUniforms**
- color : vec3

### Uniform Instances
- `colorData` : `ColorUniforms`

### Functions
- `Color() : vec4`

---

## DiscreteCurve Module

### Preprocessor Macros
- `DISCRETE_CURVE_CTRL_POINTS_SSBO`
- `"DISCRETE_CURVE_CTRL_POINTS_SSBO`

---

## Material Module

### Structs
**MaterialUniforms**
- diffuseColorTex : vec4
- specularColorTex : vec4
- ambientColorEmissionTex : vec4
- shininess : float
- hasNormalTex : int

### Uniform Instances
- `materialDiffuseTex` : `sampler2D`
- `materialSpecularTex` : `sampler2D`
- `materialEmissionTex` : `sampler2D`
- `materialNormalTex` : `sampler2D`
- `materialData` : `MaterialUniforms`

---

