varying vec2 vUv;
uniform float u_TextureGridMode;
uniform float u_TextureColorMode;
in float path;

const vec4 plainColor = vec4(1.0);

void main() {

    if (path < 0.0)
    {
        gl_FragColor = vec4(1.0, 0.68, 0.0, 1.0);
    }
    else{
        gl_FragColor = vec4(0.0,1.0,0.0,1.0);
    }
}

