#version 430 core

void main () {
	// Intentionally left blank (The gl_FragCoord.z value is written to the GL_DEPTH_ATTACHMENT automaticly)
    // gl_FragDepth = gl_FragCoord.z; // default

    // gl_FragDepth = 1.f;

    /*
    float depth = gl_FragCoord.z;
    if (depth > 0.0) {
        // Eltoljuk és felerõsítjük a különbséget
        // Ha így látsz szürke foltokat, akkor csak a távolság és a skálázás miatt volt fehér minden
        float visualDepth = (depth - 0.9) * 10.0; 
        gl_FragDepth = visualDepth; 
    }
    */
}