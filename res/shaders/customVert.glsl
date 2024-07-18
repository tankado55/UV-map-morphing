attribute float pathVerse;
varying vec2 vUv;
out float path;

    void main() {

        vUv = uv;
        gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
        path = pathVerse;
    }