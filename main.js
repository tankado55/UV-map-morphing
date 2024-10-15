import * as THREE from 'three';

import { OBJLoader } from 'three/addons/loaders/OBJLoader.js';
import { FBXLoader } from 'three/addons/loaders/FBXLoader.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

const PI = 3.141592653589793;

let camera, scene, renderer;
let cameraRTT, sceneRTT;

let object;
let heapGeometryPointer;
let heapUVPointer;
let heapPathVersesPointer;
let heapGeometryPointInterpolated;
let meshInstance;

let objPath = "res/models/"
let objSelect = document.getElementById("meshSelect");
let texturePath = "res/models/"
objPath = objPath + JSON.parse(objSelect.value).model;
texturePath = texturePath + JSON.parse(objSelect.value).tex;
console.log("texturePath: " + texturePath)
var event = new Event('change', { 'bubbles': true });
objSelect.onchange = function () {
	console.log("select onChange");
	let objSelect = document.getElementById("meshSelect");
	objPath = "res/models/" + JSON.parse(objSelect.value).model;
	texturePath = "res/models/" + JSON.parse(objSelect.value).tex;
	deallocateHeap();
	initLoadModel();

};
let rtTextureTarget;
let basicMaterial;
let material;
let customShaderMaterial;
let plane;
let quad;

let customFragShader
let customVertexShader

let defaultMaterial = new THREE.ShaderMaterial()

async function loadShaders() {
	customVertexShader = await (await fetch("res/shaders/customVert.glsl")).text();
	customFragShader = await (await fetch("res/shaders/customFrag.glsl")).text();
	customShaderMaterial = new THREE.ShaderMaterial({
		vertexShader: customVertexShader,
		fragmentShader: customFragShader
	});

}

loadShaders();

//UI
var interpolationSlider = document.getElementById("interpolationSlider");
var gridSlider = document.getElementById("gridSlider");
var whiteSlider = document.getElementById("whiteSlider");
var splitResidualElement = document.getElementById("splitResidual");
var splitResidual = splitResidualElement.value;
var shortestElement = document.getElementById("shortestPath");
var debugIslandElement = document.getElementById("debugIsland");
var debugIsland = debugIslandElement.value;
//glued
var gluedElement = document.getElementById("glued");
var glued = gluedElement.value;
var gluedModElement = document.getElementById("gluedMod");
var gluedWeightedElement = document.getElementById("gluedWeighted");
var gluingThresholdElement = document.getElementById("gluingThreshold");
var arapElement = document.getElementById("arap");
// Linear
var linearElement = document.getElementById("linear");
linearElement.checked = false;
var linear = linearElement.value;
var temporizeElement = document.getElementById("temporize");
var flightTimeElement = document.getElementById("flightTime");
var bakingElement = document.getElementById("baking");

var glueBtn = document.getElementById("glueBtn");
var unglueBtn = document.getElementById("unglueBtn");
var arapBtn = document.getElementById("arapBtn");

glueBtn.onclick = function () {
	meshInstance.glueTrianglesWeighted();
	object.traverse(function (child) {
		if (child.isMesh) {
			let arr = child.geometry.attributes.position.array;
			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
		}
	})

	render();
}
unglueBtn.onclick = function () {
	meshInstance.unglue();
	object.traverse(function (child) {
		if (child.isMesh) {
			let arr = child.geometry.attributes.position.array;
			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
		}
	})
	render();
}
arapBtn.onclick = function () {
	meshInstance.glueTriangleArap();
	object.traverse(function (child) {
		if (child.isMesh) {
			let arr = child.geometry.attributes.position.array;
			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
		}
	})
	render();
}

function updateUI() {
	linearElement.dispatchEvent(event);
	splitResidualElement.dispatchEvent(event);
	shortestElement.dispatchEvent(event);
	gluedModElement.dispatchEvent(event);
	debugIslandElement.dispatchEvent(event);
	temporizeElement.dispatchEvent(event);
	gluedWeightedElement.dispatchEvent(event);
}

interpolationSlider.oninput = function () {
	interpolate();
	render();
}
gridSlider.oninput = function () {
	material.uniforms.u_TextureGridMode.value = parseFloat(gridSlider.value) / 100.0;
	renderQuadTexture()
	interpolate()
	render();
}
whiteSlider.oninput = function () {
	material.uniforms.u_TextureColorMode.value = parseFloat(whiteSlider.value) / 100.0;
	renderQuadTexture();
	interpolate()
	render();
}

splitResidualElement.onchange = function () {
	splitResidual = splitResidualElement.checked;
	interpolate()
	render();
}
linearElement.onchange = function () {
	linear = linearElement.checked;
	shortestElement.disabled = linear;
	splitResidualElement.disabled = linear;
	gluedElement.disabled = linear;
	gluedModElement.disabled = linear;
	gluedWeightedElement.disabled = linear;
	interpolate()
	render();
}
shortestElement.onchange = function () {
	if (shortestElement.value < 3) {
		meshInstance.updatePathVerse(parseInt(shortestElement.value));
	}
	else {
		meshInstance.updatePathVersePerIsland();
	}
	interpolate()
	render();
}
gluedElement.onchange = function () {
	meshInstance.glued = gluedElement.checked;
	if (!gluedElement.checked) {
		gluedModElement.disabled = true;
	}
	else {
		gluedModElement.disabled = false;
	}
	interpolate()
	render();
}
arapElement.onchange = function () {
	if (arapElement.checked)
		meshInstance.precomputeARAP();
	interpolate()
	render();
}
gluedWeightedElement.onchange = function () {
	meshInstance.weightedGluing = gluedWeightedElement.checked
	interpolate()
	render();
}

gluedModElement.onchange = function () {
	meshInstance.updateCopyOf(parseInt(gluedModElement.value));
	interpolate()
	render();
}
debugIslandElement.onchange = function () {
	debugIsland = debugIslandElement.checked;
	object.traverse(function (child) {

		if (child.isMesh) {
			if (!debugIsland) {
				child.material = new THREE.MeshStandardMaterial({ map: rtTextureTarget.texture })
				child.material.side = THREE.DoubleSide;
				child.castShadow = true;
				// child.receiveShadow = true;
				//child.material.shadowSide = THREE.FrontSide;
			}
			else
				child.material = customShaderMaterial;
		}

	});
	renderQuadTexture()
	interpolate()
	render();
}
temporizeElement.onchange = function () {
	var value = temporizeElement.value;
	if (value == "no")
		meshInstance.resetTiming();
	else if (value == "u") {
		meshInstance.setTimingWithU(parseFloat(flightTimeElement.value))
	}
	else if (value == "v") {
		meshInstance.setTimingWithV(parseFloat(flightTimeElement.value))
	}
	else if (value == "insideO") {
		meshInstance.setTimingInsideOut(parseFloat(flightTimeElement.value))
	}
	interpolate()
	render();
}
gluingThresholdElement.oninput = function () {
	meshInstance.setGluingThreshold(parseFloat(gluingThresholdElement.value));
	if (parseInt(gluingThresholdElement.value) == 1) {
		meshInstance.updateCopyOfUsingThreshold(parseInt(gluedModElement.value));
	}
	interpolate()
	render();
}

bakingElement.onchange = function () {
	if (bakingElement.checked) {
		meshInstance.bake(101, splitResidual, linear, gluedElement.checked, arapElement.checked)
	}
}

/////////////////////////////////////////*** Init */

initRTT()
init();
Module.onRuntimeInitialized = () => { initLoadModel(); }
initHorPlane();

function initHorPlane() {
	const geometry = new THREE.PlaneGeometry(10, 10);
	const material = new THREE.MeshStandardMaterial({ color: 0xffffff });
	const plane = new THREE.Mesh(geometry, material);
	plane.translateY(-2.0);
	plane.rotateX(-PI / 2);
	plane.receiveShadow = true;
	//scene.add(plane);
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
	cameraRTT = new THREE.OrthographicCamera(window.innerWidth / - 2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / - 2, - 10000, 10000);
	cameraRTT.position.z = 100;
	rtTextureTarget = new THREE.WebGLRenderTarget(window.innerWidth, window.innerHeight);
	material = new THREE.ShaderMaterial({

		uniforms: { tDiffuse: { value: null }, u_TextureGridMode: { value: parseFloat(gridSlider.value) / 100.0 }, u_TextureColorMode: { value: parseFloat(whiteSlider.value) / 100.0 } },
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

function deallocateHeap() {
	scene.remove(object);
	Module._free(heapGeometryPointer);
	Module._free(heapPathVersesPointer);
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

	const dirLight = new THREE.DirectionalLight(0xffffff, 1);
	dirLight.position.set(0, 5, 0); //default; light shining from top
	dirLight.castShadow = true; // default false
	scene.add(dirLight);

	//

	const controls = new OrbitControls(camera, renderer.domElement);
	controls.minDistance = 2;
	controls.maxDistance = 160;
	controls.addEventListener('change', render);

	//

	window.addEventListener('resize', onWindowResize);
	console.log(scene)
}

function initLoadModel() {

	function loadModel() {

		// heap pointer
		object.traverse(function (child) {

			if (child.isMesh) {
				let arr = child.geometry.attributes.position.array;
				let arrUV = child.geometry.attributes.uv.array;
				let pathVerse = new Float32Array(child.geometry.attributes.position.count)
				for (let i = 0; i < child.geometry.attributes.position.count; i++) {
					pathVerse[i] = 5.5;
				}
				child.geometry.setAttribute('pathVerse', new THREE.BufferAttribute(pathVerse, 1))

				let arrPathVerses = child.geometry.attributes.pathVerse.array;

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
					heapPathVersesPointer = transferNumberArrayToHeap(
						arrPathVerses,
						hasFloatValue ? TYPES.f32 : TYPES.f32 // forcing f32
					);
					meshInstance = new Module.Mesh(heapGeometryPointer, heapUVPointer, heapPathVersesPointer, arr.length, arrUV.length);
				}
				catch (error) {
					console.log("error in transfer to heap")
					console.log(error)
				}

				child.geometry.computeBoundingSphere();
				let radius = child.geometry.boundingSphere.radius
				object.scale.set(1.0 / radius, 1.0 / radius, 1.0 / radius);
				child.scale.set(1.0, 1.0, 1.0);
				child.rotation.x = 0;
				child.rotation.y = 0;
				child.rotation.z = 0;
				child.position.set(0, 0, 0);
				console.log(child.geometry)
				console.log(child)

				meshInstance.glued = gluedElement.checked


				updateUI();

			}
		});
		interpolate()
		render();

	}


	const manager = new THREE.LoadingManager(loadModel);

	// texture

	const textureLoader = new THREE.TextureLoader();
	textureLoader.load(
		texturePath,

		// onLoad callback
		function (texture) {
			material.uniforms.tDiffuse.value = texture;

		},

		// onProgress
		undefined,

		function (err) {
			console.error('Error in loading texture');
			console.log(err)
			material.uniforms.tDiffuse.value = null;
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
	if (extension == "obj") {
		loader = new OBJLoader(manager);
	}
	else {
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
				if (!debugIsland) {
					child.material = new THREE.MeshStandardMaterial({ map: rtTextureTarget.texture })
					// var geo = new THREE.EdgesGeometry( child.geometry );
					// var mat = new THREE.LineBasicMaterial( { color: 0xffffff, linewidth: 2 } );
					// child.geometry = geo
					// child.material = mat

				}
				else
					child.material = customShaderMaterial;

				child.material.side = THREE.DoubleSide;
				child.castShadow = true;
				//child.receiveShadow = true;
				//child.material.shadowSide = THREE.FrontSide;
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
	console.log(typedArray.length)

	Module[type.heap].set(typedArray, heapPointer >> 2); // copy, This aligns the memory pointer to a 4-byte boundary, which is often necessary for proper memory access in low-level languages like C or C++.

	return heapPointer;
}

function containsFloatValue(array) {
	return array.some((value) => !Number.isInteger(value));
}

function onWindowResize() {

	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();

	renderer.setSize(window.innerWidth, window.innerHeight);
}

function interpolate() {
	object.traverse(function (child) {
		if (child.isMesh) {
			console.log("Interpolating: " + interpolationSlider.value)

			let arr = child.geometry.attributes.position.array;
			if (bakingElement.checked) {
				meshInstance.applyBaked(parseInt(interpolationSlider.value));
			}
			else {
				meshInstance.interpolatePerTriangle(parseInt(interpolationSlider.value), splitResidual, linear);
				if (gluedElement.checked) {
					if (parseInt(gluingThresholdElement.value) < 1) {
						meshInstance.updateCopyOfUsingThreshold(parseInt(gluedModElement.value));
					}
					if (arapElement.checked) {
						meshInstance.glueTriangleArap()
					}
					else if (gluedWeightedElement.checked) {
						meshInstance.glueTrianglesWeighted()
					}
					else {
						meshInstance.glueTriangles();
					}
				}
			}



			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
			child.geometry.setAttribute('pathVerse', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapPathVersesPointer >> 2, (heapPathVersesPointer >> 2) + child.geometry.attributes.position.length), 1));
			child.geometry.attributes.pathVerse.needsUpdate = true;
			child.geometry.computeVertexNormals()
		}

	});
}

function render() {
	renderer.clear();
	renderer.render(scene, camera);
}
