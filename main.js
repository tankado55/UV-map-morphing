import * as THREE from 'three';

import { OBJLoader } from 'three/addons/loaders/OBJLoader.js';
import { FBXLoader } from 'three/addons/loaders/FBXLoader.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

const PI = 3.141592653589793;

//var int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
//console.log(int_sqrt(300))

let camera, scene, renderer;
let cameraRTT
let sceneRTT
let sceneScreen

let object;
let heapGeometryPointer;
let heapUVPointer;
let heapGeometryPointInterpolated;
let meshInstance;

let objPath = "res/models/"
let objSelect = document.getElementById("meshSelect");
objPath = objPath + objSelect.value;
objSelect.onchange = function () {
	console.log("select onChange");
	let objSelect = document.getElementById("meshSelect");
	objPath = "res/models/" + objSelect.value;
	deallocateHeap();
	initLoadModel();
};
let texturePath = "res/models/_Wheel_195_50R13x10_OBJ/diffuse.png"
let texture;
let rtTextureTarget;
let basicMaterial;
let material;
let plane;
let quad;
let dirLight;

//UI
var interpolationSlider = document.getElementById("interpolationSlider");
var gridSlider = document.getElementById("gridSlider");
var whiteSlider = document.getElementById("whiteSlider");
var splitResidualElement = document.getElementById("splitResidual");
var splitResidual = splitResidualElement.value;
var linearElement = document.getElementById("linear");
var linear = linearElement.value;
var shortestElement = document.getElementById("shortestPath");
var shortestPath = shortestElement.value;
//glued
var gluedElement = document.getElementById("glued");
var glued = gluedElement.value;
var gluedModElement = document.getElementById("gluedMod");
var gluedMod = gluedModElement.value;

interpolationSlider.oninput = function () {
	render();
}
gridSlider.oninput = function () {
	material.uniforms.u_TextureGridMode.value = parseFloat(gridSlider.value) / 100.0;
	renderQuadTexture()
	render();
}
whiteSlider.oninput = function () {
	material.uniforms.u_TextureColorMode.value = parseFloat(whiteSlider.value) / 100.0;
	renderQuadTexture()
	render();
}

splitResidualElement.onchange = function () {
	splitResidual = splitResidualElement.checked;
	render();
}
linearElement.onchange = function () {
	linear = linearElement.checked;
	render();
}
shortestElement.onchange = function () {
	shortestPath = shortestElement.checked;
	render();
}
gluedElement.onchange = function () {
	glued = gluedElement.checked;
	if (!glued) {
		gluedModElement.disabled = true;
	}
	else {
		gluedModElement.disabled = false;
	}
	render();
}
gluedModElement.onchange = function () {
	gluedMod = gluedModElement.value;
	meshInstance.updateCopyOf(gluedMod);
	render();
}


initRTT()
init();
initLoadModel();
initHorPlane();

console.log(meshInstance)

function initHorPlane() {
	const geometry = new THREE.PlaneGeometry(10, 10);
	const material = new THREE.MeshStandardMaterial({ color: 0xfdfdfd, side: THREE.DoubleSide });
	const plane = new THREE.Mesh(geometry, material);
	plane.translateY(-3.0);
	plane.rotateX(- PI/2);
	plane.receiveShadow = true;
	scene.add(plane);
}

function renderQuadTexture() {
	renderer.setRenderTarget(rtTextureTarget);
	renderer.clear();
	renderer.render(sceneRTT, cameraRTT);

	renderer.setRenderTarget(null);
}


function initRTT() {
	renderer = new THREE.WebGLRenderer({ antialias: true });
	renderer.setPixelRatio(window.devicePixelRatio);
	renderer.setSize(window.innerWidth, window.innerHeight);
	renderer.setClearColor(0xffffff, 0);
	renderer.autoClear = false;
	renderer.shadowMap.enabled = true;
	renderer.shadowMap.type = THREE.PCFSoftShadowMap;
	document.body.appendChild(renderer.domElement);

	sceneRTT = new THREE.Scene();
	sceneScreen = new THREE.Scene();
	cameraRTT = new THREE.OrthographicCamera(window.innerWidth / - 2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / - 2, - 10000, 10000);
	cameraRTT.position.z = 100;
	rtTextureTarget = new THREE.WebGLRenderTarget(window.innerWidth, window.innerHeight);
	material = new THREE.ShaderMaterial({

		uniforms: { tDiffuse: { value: texture }, u_TextureGridMode: { value: parseFloat(gridSlider.value) / 100.0 }, u_TextureColorMode: { value: parseFloat(whiteSlider.value) / 100.0 } },
		//uniforms: { tDiffuse: { value: texture } },
		vertexShader: document.getElementById('vertexshader').textContent,
		fragmentShader: document.getElementById('fragmentshader').textContent

	});
	plane = new THREE.PlaneGeometry(window.innerWidth, window.innerHeight);

	quad = new THREE.Mesh(plane, material);
	quad.position.z = - 100;
	sceneRTT.add(quad);

	renderer.setRenderTarget(rtTextureTarget);
	renderer.clear();
	renderer.render(sceneRTT, cameraRTT);

	renderer.setRenderTarget(null);
}

function deallocateHeap()
	{
		scene.remove(object);
		Module._free(heapGeometryPointer);
		Module._free(heapUVPointer);
		Module._free(heapGeometryPointInterpolated);
		meshInstance.delete();
	}

function init() {

	scene = new THREE.Scene();

	camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 200);
	camera.position.z = 20.0;
	scene.add(camera);

	const ambientLight = new THREE.AmbientLight(0xffffff);
	scene.add(ambientLight);

	//const pointLight = new THREE.PointLight(0xffffff, 15);
	//camera.add(pointLight);

	const dirLight = new THREE.DirectionalLight( 0xffffff, 1 );
	dirLight.position.set( 0, 5, 0 ); //default; light shining from top
	dirLight.castShadow = true; // default false
	scene.add( dirLight );

	//

	const controls = new OrbitControls(camera, renderer.domElement);
	controls.minDistance = 2;
	controls.maxDistance = 160;
	controls.addEventListener('change', render);

	//

	window.addEventListener('resize', onWindowResize);
	console.log(scene)
}

function initLoadModel()
{
	// manager
	function loadModel() {

		// heap pointer
		object.traverse(function (child) {

			if (child.isMesh) {
				let arr = child.geometry.attributes.position.array;
				let arrUV = child.geometry.attributes.uv.array;

				const hasFloatValue = containsFloatValue(arr);
				try {
					heapGeometryPointer = transferNumberArrayToHeap(
						arr,
						hasFloatValue ? TYPES.f32 : TYPES.i32
					);
					heapGeometryPointInterpolated = transferNumberArrayToHeap(
						arr,
						hasFloatValue ? TYPES.f32 : TYPES.i32
					);
					heapUVPointer = transferNumberArrayToHeap(
						arrUV,
						hasFloatValue ? TYPES.f32 : TYPES.i32
					);
					meshInstance = new Module.Mesh(heapGeometryPointer, heapUVPointer, arr.length, arrUV.length);
					console.log("boundingsphere")
					//console.log(meshInstance.boundingSphere);
					console.log(Module["HEAPF32"][heapGeometryPointer >> 2])
				}
				catch (error) {
					console.log("error in transfer to heap")
					console.log(error)
				}

				child.geometry.computeBoundingSphere();
				let radius = child.geometry.boundingSphere.radius
				object.scale.set(1.0 / radius, 1.0 / radius, 1.0 / radius);
				child.scale.set(1.0,1.0,1.0);
				child.rotation.x = 0;
				child.rotation.y = 0;
				child.rotation.z = 0;
				child.position.set(0,0,0);
				console.log(radius)
				console.log(object.scale)
				console.log(child.geometry)

				// debug
				let sphGeometry = new THREE.SphereGeometry( radius, 32, 16 );
				sphGeometry = new THREE.WireframeGeometry( sphGeometry );
				const sphMaterial = new THREE.MeshBasicMaterial( { color: 0xffff00 } ); 
				const sphere = new THREE.Mesh( sphGeometry, sphMaterial );
				//sphere.scale.set(1.0 / radius, 1.0 / radius, 1.0 / radius)
				console.log(object)
				//scene.add( sphere );

			}
		});
	}


	const manager = new THREE.LoadingManager(loadModel);

	// texture

	const textureLoader = new THREE.TextureLoader();
	textureLoader.load(
		texturePath,

		// onLoad callback
		function (texture) {
			// in this example we create the material when the texture is loaded
			material.uniforms.tDiffuse.value = texture;

		},

		// onProgress
		undefined,

		// onError callback
		function (err) {
			console.error('An error happened.');
		}
	);


	// model

	function onProgress(xhr) {

		if (xhr.lengthComputable) {

			const percentComplete = xhr.loaded / xhr.total * 100;
			console.log('model ' + percentComplete.toFixed(2) + '% downloaded');

		}

	}

	let extension = objPath.split('.')[1];
	let loader;
	if (extension == "obj")
	{
		loader = new OBJLoader(manager);
	}
	else{
		loader = new FBXLoader(manager);
	}
	
	loader.load(objPath, function (obj) {
		renderer.setRenderTarget(rtTextureTarget);
		renderer.clear();
		renderer.render(sceneRTT, cameraRTT);

		renderer.setRenderTarget(null);

		object = obj;
		obj.traverse(function (child) {

			if (child.isMesh) {
				child.material = new THREE.MeshStandardMaterial({ map: rtTextureTarget.texture })
				child.material.side = THREE.DoubleSide;
				child.castShadow = true;
				child.receiveShadow = true;
			}

		});
		obj.position.y = 0.0;
		scene.add(obj);
	}, onProgress, function () { console.log("Error") });
}

const TYPES = {
	i8: { array: Int8Array, heap: "HEAP8" },
	i16: { array: Int16Array, heap: "HEAP16" },
	i32: { array: Int32Array, heap: "HEAP32" },
	f32: { array: Float32Array, heap: "HEAPF32" },
	f64: { array: Float64Array, heap: "HEAPF64" },
	u8: { array: Uint8Array, heap: "HEAPU8" },
	u16: { array: Uint16Array, heap: "HEAPU16" },
	u32: { array: Uint32Array, heap: "HEAPU32" }
};

function transferNumberArrayToHeap(array, type) {
	const typedArray = type.array.from(array); // shallow
	const heapPointer = Module._malloc(typedArray.length * typedArray.BYTES_PER_ELEMENT);
	console.log(typedArray.length) //9k float

	Module[type.heap].set(typedArray, heapPointer >> 2); // copy, This aligns the memory pointer to a 4-byte boundary, which is often necessary for proper memory access in low-level languages like C or C++.

	return heapPointer;
}

function containsFloatValue(array) {
	return array.some((value) => !Number.isInteger(value));
}
/*
function plusOne() {
	object.traverse(function (child) {

		if (child.isMesh) {
			child.geometry.dynamic = true;
			let arr = child.geometry.attributes.position.array;
			//console.log(arr)

			console.log("js, bef: " + arr[0])

			const hasFloatValue = containsFloatValue(arr);
			let pointerToArrayOnHeap;
			try {
				pointerToArrayOnHeap = transferNumberArrayToHeap(
					arr,
					hasFloatValue ? TYPES.f32 : TYPES.i32
				);
				Module.ccall(
					"plusOne", // The name of C++ function
					null, // The return type
					["number", "number"], // The argument types
					[pointerToArrayOnHeap, arr.length] // The arguments
				);
				//child.geometry.attributes.position.array = Module["HEAPF32"].slice(pointerToArrayOnHeap >> 2, (pointerToArrayOnHeap >> 2) + arr.length)
				child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(pointerToArrayOnHeap >> 2, (pointerToArrayOnHeap >> 2) + arr.length), 3));
				child.geometry.verticesNeedUpdate = true;
			}
			finally {
				console.log(Module["HEAPF32"][pointerToArrayOnHeap >> 2])
				Module._free(pointerToArrayOnHeap);
			}
		}

	});
}*/

function plusOne() {
	object.traverse(function (child) {
		if (child.isMesh) {
			let arr = child.geometry.attributes.position.array;
			Module.ccall(
				"plusOne", // The name of C++ function
				null, // The return type
				["number", "number"], // The argument types
				[heapGeometryPointer, arr.length] // The arguments
			);
			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
		}

	});
}

function interpolate() {
	object.traverse(function (child) {
		if (child.isMesh) {

			let arr = child.geometry.attributes.position.array;
			/*
			Module.ccall(
				"interpolate", // The name of C++ function
				null, // The return type
				["number", "number", "number", "number", "number"], // The argument types
				[heapGeometryPointer, heapUVPointer, slider.value, arr.length, heapGeometryPointInterpolated] // The arguments
			);
			*/
			//meshInstance.interpolate(parseInt(interpolationSlider.value))
			meshInstance.interpolatePerTriangle(parseInt(interpolationSlider.value), splitResidual, linear, shortestPath);
			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
		}

	});
}

function onWindowResize() {

	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();

	renderer.setSize(window.innerWidth, window.innerHeight);
}

function render() {
	interpolate();
	renderer.clear();
	renderer.render(scene, camera);
}
