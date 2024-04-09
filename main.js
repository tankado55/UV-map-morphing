import * as THREE from 'three';

import { OBJLoader } from 'three/addons/loaders/OBJLoader.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

//var int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
//console.log(int_sqrt(300))

let camera, scene, renderer;

let object;
let heapGeometryPointer;
let heapUVPointer;
let heapGeometryPointInterpolated;
let meshInstance;

init();

var slider = document.getElementById("myRange");
slider.oninput = function() {
    render();
}

function init() {

	camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 20);
	camera.position.z = 2.5;

	// scene

	scene = new THREE.Scene();

	const ambientLight = new THREE.AmbientLight(0xffffff);
	scene.add(ambientLight);

	const pointLight = new THREE.PointLight(0xffffff, 15);
	camera.add(pointLight);
	scene.add(camera);

	// manager

	function loadModel() {

		object.traverse(function (child) {

			if (child.isMesh) {
				child.material.map = texture;
				console.log(texture)
				console.log( child.geometry);
				//console.log( child.geometry.attributes.position.array);
			}

		});

		object.position.y = 0.0;
		object.scale.setScalar(0.01);
		scene.add(object);

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
					console.log(meshInstance);
					console.log(meshInstance.debugInt);
					console.log(Module["HEAPF32"][heapGeometryPointer >> 2])
				}
				catch (error) {
					console.log("error in transfer to heap")
					console.log(error)
				}
			}
		});

		render();

	}



	const manager = new THREE.LoadingManager(loadModel);

	// texture

	const textureLoader = new THREE.TextureLoader(manager);
	const texture = textureLoader.load('textures/uv_grid_opengl.jpg', render);
	texture.colorSpace = THREE.SRGBColorSpace;

	// model

	function onProgress(xhr) {

		if (xhr.lengthComputable) {

			const percentComplete = xhr.loaded / xhr.total * 100;
			console.log('model ' + percentComplete.toFixed(2) + '% downloaded');

		}

	}

	function onError() { }

	const loader = new OBJLoader(manager);
	loader.load('models/wheel.obj', function (obj) {

		object = obj;
		//console.log(object)

	}, onProgress, onError);

	//

	renderer = new THREE.WebGLRenderer({ antialias: true });
	renderer.setPixelRatio(window.devicePixelRatio);
	renderer.setSize(window.innerWidth, window.innerHeight);
	renderer.setClearColor( 0xffffff, 0);
	document.body.appendChild(renderer.domElement);

	//

	const controls = new OrbitControls(camera, renderer.domElement);
	controls.minDistance = 2;
	controls.maxDistance = 5;
	controls.addEventListener('change', render);

	//

	window.addEventListener('resize', onWindowResize);
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
	const heapPointer = Module._malloc(
		typedArray.length * typedArray.BYTES_PER_ELEMENT
	);

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

function interpolate()
{
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
			meshInstance.interpolate(parseInt(slider.value))
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

	//plusOne();
	interpolate();
	renderer.render(scene, camera);
	console.log(slider.value)
}
