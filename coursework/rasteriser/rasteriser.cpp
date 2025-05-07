// This define is necessary to get the M_PI constant.
#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <lodepng.h>
#include "Image.hpp"
#include "LinAlg.hpp"
#include "Light.hpp"
#include "Mesh.hpp"

// ***** WEEK 6 LAB *****
// Subtask 1: Implement the projectionMatrix function, to make a projection matrix to view your scene!
// Subtask 2: Complete the transformation chain, moving vertices from model space all the way to screen space.
// Subtask 3: Set up the camera and projection matrices for the transformation chain
// Subtask 4: Implement Z buffering.
// Subtask 5: Implement texture mapping.
// If you finish early - note that we now have all the tools to properly set up your own scene!
// This is a great time to start on your own code in the coursework/rasteriser folder, using this as a base if
// you wish. We will in future labs work on more advanced shading, but you can port this feature over later.

struct Triangle {
	std::array<Eigen::Vector3f, 3> screen; // Coordinates of the triangle in screen space.
	std::array<Eigen::Vector3f, 3> verts; // Vertices of the triangle in world space.
	std::array<Eigen::Vector3f, 3> norms; // Normals of the triangle corners in world space.
	std::array<Eigen::Vector2f, 3> texs; // Texture coordinates of the triangle corners.
};


Eigen::Matrix4f projectionMatrix(int height, int width, float horzFov = 100.f * M_PI / 180.f, float zFar = 5000.f, float zNear = 0.1f)
{
	// ========= Subtask 1: Make a Projection Matrix ========
	// *** YOUR CODE HERE ***

	// Make a projection matrix following the formulation in the lecture slides, and using the provided parameters.
	// First, work out vertical FoV based on the horizontal FoV:
	float vertFov = (horzFov * height) / width;
	// Now construct the matrix.
	Eigen::Matrix4f projection;
	projection << 1 / (tan(horzFov / 2)), 0, 0, 0,
		0, 1 / (tan(vertFov / 2)), 0, 0,
		0, 0, zFar / (zFar - zNear), -(zFar * zNear) / (zFar - zNear),
		0, 0, 1, 0;

	return projection;
	// *** END YOUR CODE ***
}

void findScreenBoundingBox(const Triangle& t, int width, int height, int& minX, int& minY, int& maxX, int& maxY)
{
	// Find a bounding box around the triangle
	int padding = 10; // Define how much to increase the bounding box

	minX = std::min(std::min(t.screen[0].x(), t.screen[1].x()), t.screen[2].x());
	minY = std::min(std::min(t.screen[0].y(), t.screen[1].y()), t.screen[2].y());
	maxX = std::max(std::max(t.screen[0].x(), t.screen[1].x()), t.screen[2].x());
	maxY = std::max(std::max(t.screen[0].y(), t.screen[1].y()), t.screen[2].y());

	// Increase the size of the bounding box by the padding
	minX -= padding;
	maxX += padding;
	minY -= padding;
	maxY += padding;

	minX = std::max(minX, 0);                
	maxX = std::min(maxX, width - 1);        
	minY = std::max(minY, 0);                
	maxY = std::min(maxY, height - 1);       
}



void drawTriangle(std::vector<uint8_t>& image, int width, int height,
	std::vector<float>& zBuffer,
	const Triangle& t,
	const std::vector<std::unique_ptr<Light>>& lights,
	const std::vector<uint8_t>& albedoTexture, int texWidth, int texHeight)
{
	int minX, minY, maxX, maxY;
	findScreenBoundingBox(t, width, height, minX, minY, maxX, maxY);

	Eigen::Vector2f edge1 = v2(t.screen[2] - t.screen[0]);
	Eigen::Vector2f edge2 = v2(t.screen[1] - t.screen[0]);
	float triangleArea = 0.5f * vec2Cross(edge2, edge1);
	if (triangleArea < 0) {
		// Triangle is backfacing
		// Exit and quit drawing!
		return;
	}

	for (int x = minX; x <= maxX; ++x)
		for (int y = minY; y <= maxY; ++y) {
			Eigen::Vector2f p(x, y);

			// Find sub-triangle areas
			float a0 = 0.5f * fabsf(vec2Cross(v2(t.screen[1]) - v2(t.screen[2]), p - v2(t.screen[2])));
			float a1 = 0.5f * fabsf(vec2Cross(v2(t.screen[0]) - v2(t.screen[2]), p - v2(t.screen[2])));
			float a2 = 0.5f * fabsf(vec2Cross(v2(t.screen[0]) - v2(t.screen[1]), p - v2(t.screen[1])));

			// find barycentrics
			float b0 = a0 / triangleArea;
			float b1 = a1 / triangleArea;
			float b2 = a2 / triangleArea;

			// If outside triangle, exit early
			float sum = b0 + b1 + b2;
			if (sum > 1.0001) {
				continue;
			}

			Eigen::Vector3f worldP = t.verts[0] * b0 + t.verts[1] * b1 + t.verts[2] * b2;

			// ========== Subtask 4: Z Buffering ==========
			// Here we'll implement Z-buffering, using the zBuffer image and working out the 
			// depth of this pixel in screen space.
			// HINT: If you have trouble with this task, note that I've added code to save the z buffer to
			// zBuffer.png. This is encoded so further away objects are lighter in color. It should match
			// the example_zBuffer.png image if your code is working!
			// *** YOUR CODE HERE ***

			// First, work out the depth of this location in screen space. 
			// We saved the clip space z values in t.screen[0].z(), t.screen[1].z() and t.screen[2].z.
			// Use barycentric interpolation on these to work out the depth of this pixel.
			float depth = (t.screen[0].z() * b0) + (t.screen[1].z() * b1) + (t.screen[2].z() * b2);

			// Work out where to sample in the zBuffer. Remember the zBuffer has only one channel,
			// so your index should be based on the pixel's x and y locations, and the width of the 
			// z buffer only.
			int depthIdx = p.y() * width + p.x();

			// If your depth is bigger than the current depth, skip drawing this pixel.
			// Otherwise, replace the zBuffer value at depthIdx with this depth.
			// ADD YOUR OWN CODE TO DO THIS HERE
			if (depth > zBuffer[depthIdx]) {
				continue;
			}
			zBuffer[depthIdx] = depth;
			// *** END YOUR CODE ***

			Eigen::Vector3f normP = t.norms[0] * b0 + t.norms[1] * b1 + t.norms[2] * b2;
			normP.normalize();



			// ========== Subtask 5: Texture Mapping ===========
			// Here we'll actually implement the texture mapping! Follow the steps below, implementing each
			// stage in turn.
			// *** YOUR CODE HERE ***
			// Add code to calculate the texture coordinates corresponding to P, texP.
			// Use barycentric interpolation!
			Eigen::Vector2f texP = (t.texs[0] * b0) + (t.texs[1] * b1) + (t.texs[2] * b2);

			// Convert this coordinate to a point in texture space
			// To do so, multiply by the texWidth and texHeight to get to the correct range.
			// Don't forget to flip the y coordinates! 
			int texR = texHeight - (texP.y() * texHeight);
			int texC = texP.x() * texWidth;
			// Handle the case where texR or texC end up outside the image!
			// There are different ways you could do this - for example using 
			// the modulo (%) operator to wrap around, or clamping to the edges.
			// Write your own code below to do this - once you're done you should be sure 
			// that 0 <= texC < texWidth and 0 <= texR < texHeight.
			if (texC < 0 || texC >= texWidth || texR < 0 || texR >= texHeight) {
				continue;
			}

			// Get the value from the texture (hint: use the getPixel function on the albedoTexture).
			Color texColor = getPixel(albedoTexture, texC, texR, texWidth, texHeight);

			//Alpha Transparency
			float alpha = texColor.a / 255.0f;
			if (texColor.a <= 1.0f) {
				continue;
			}

			Color dstColor = getPixel(image, x, y, width, height);

			Eigen::Vector3f dst(
				dstColor.r / 255.0f,
				dstColor.g / 255.0f,
				dstColor.b / 255.0f
			);

			// Convert it into an Eigen::Vector3f as an albedo
			// (Optional bonus task, if you checked out the slides on gamma correction:
			// gamma correct this colour, so the texture doesn't appear overly bright.
			// should you raise to the power 1/2.2, or 2.2?)
			Eigen::Vector3f albedo;
			albedo.x() = texColor.r / 255.0f;
			albedo.y() = texColor.g / 255.0f;
			albedo.z() = texColor.b / 255.0f;

			// *** END YOUR CODE ***


			// ----- Lighting code ------
			// Work out colour at this position.
			Eigen::Vector3f color = Eigen::Vector3f::Zero();

			// Iterate over lights, and sum to find colour.
			for (auto& light : lights) {

				// Work out the contribution from this light source, and add it to the color variable.

				// Work out the intensity of this light source, at the point worldP.
				Eigen::Vector3f lightIntensity = light->getIntensityAt(worldP);

				// We only need to do the following if the light isn't an ambient light.
				if (light->getType() != Light::Type::AMBIENT) {

					// Take the dot product of the normal with the light direction.
					float dotProd = normP.dot(-light->getDirection(worldP));

					// We don't want negative light - if dot product less than 0, set it to 0.
					dotProd = std::max(dotProd, 0.0f);

					// Multiply the light intensity by the dot product.
					lightIntensity *= dotProd;
				}

				// Now add the intensity times the albedo.
				color += coeffWiseMultiply(lightIntensity, albedo);
			}

			Color c;
			// Gamma-correcting colours.
			c.r = std::min(powf(color.x(), 1 / 2.2f), 1.0f) * 255;
			c.g = std::min(powf(color.y(), 1 / 2.2f), 1.0f) * 255;
			c.b = std::min(powf(color.z(), 1 / 2.2f), 1.0f) * 255;

			c.a = 255;

			setPixel(image, x, y, width, height, c);
		}
}

void drawMesh(std::vector<unsigned char>& image,
	std::vector<float>& zBuffer,
	const Mesh& mesh,
	const std::vector<uint8_t>& albedoTexture, int texWidth, int texHeight,
	const Eigen::Matrix4f& modelToWorld,
	const Eigen::Matrix4f& worldToClip,
	const std::vector<std::unique_ptr<Light>>& lights,
	int width, int height, bool isHighRes = false)
{

	int renderWidth = isHighRes ? width * 2 : width;
	int renderHeight = isHighRes ? height * 2 : height;

	std::vector<unsigned char> tempImage(renderWidth * renderHeight * 4, 0);  // RGBA format
	std::vector<float> tempZBuffer(renderWidth * renderHeight, std::numeric_limits<float>::infinity());


	for (int i = 0; i < mesh.vFaces.size(); ++i) {


		Eigen::Vector3f
			v0 = mesh.verts[mesh.vFaces[i][0]],
			v1 = mesh.verts[mesh.vFaces[i][1]],
			v2 = mesh.verts[mesh.vFaces[i][2]];
		Eigen::Vector3f
			n0 = mesh.norms[mesh.nFaces[i][0]],
			n1 = mesh.norms[mesh.nFaces[i][1]],
			n2 = mesh.norms[mesh.nFaces[i][2]];

		Triangle t;
		t.verts[0] = (modelToWorld * vec3ToVec4(v0)).block<3, 1>(0, 0);
		t.verts[1] = (modelToWorld * vec3ToVec4(v1)).block<3, 1>(0, 0);
		t.verts[2] = (modelToWorld * vec3ToVec4(v2)).block<3, 1>(0, 0);

		// ======= Subtask 2: The Transformation Chain ======
		//*** YOUR CODE HERE ***
		// We've worked out the vertices in *world* space above.
		// You need to do the rest of the transformation chain!
		// Work out the vClip vectors, which are the vectors in clip space
		// Multiply by worldToClip, and do the perspective divide by the w component.
		// Check that all 3 vertices are in the clip box (-1 to 1 in x, y and z) and if not,
		// skip drawing this triangle.
		// Hint: use the outsideClipBox function to do this.
		// Finally, work out the screen space coordinates based on the image height and width.

		// Work out the clip space coordinates, by multiplying by worldToClip and doing the 
		// perspective divide.
		Eigen::Vector4f vClip0 = (worldToClip * vec3ToVec4(t.verts[0]));
		Eigen::Vector4f vClip1 = (worldToClip * vec3ToVec4(t.verts[1]));
		Eigen::Vector4f vClip2 = (worldToClip * vec3ToVec4(t.verts[2]));

		vClip0 = vClip0 / vClip0[3];
		vClip1 = vClip1 / vClip1[3];
		vClip2 = vClip2 / vClip2[3];

		std::vector<Eigen::Vector4f> clipVerts{ vClip0, vClip1, vClip2 };

		// Check that all 3 vertices are in the clip box (-1 to 1 in x, y and z) and if not,
		// skip drawing this triangle.
		// Hint: I've made a function outsideClipBox in LinAlg.hpp to help with this!

		bool isVaild = true;
		for (int j = 0; j < 3; j++) {
			if (outsideClipBox(clipVerts[j])) {
				isVaild = false;
			}
		}
		if (isVaild) {

			// Work out the screen space coordinates based on the image height and width.
			// Set the z component of each screen coordinate to be the clip-space z (for example
			// t.screen[0].z() == vClip0.z());
			t.screen[0].x() = width * (vClip0.x() + 1) / 2;
			t.screen[0].y() = height * (-vClip0.y() + 1) / 2;
			t.screen[0].z() = vClip0.z();
			t.screen[1].x() = width * (vClip1.x() + 1) / 2;
			t.screen[1].y() = height * (-vClip1.y() + 1) / 2;
			t.screen[1].z() = vClip1.z();
			t.screen[2].x() = width * (vClip2.x() + 1) / 2;
			t.screen[2].y() = height * (-vClip2.y() + 1) / 2;
			t.screen[2].z() = vClip2.z();
			// *** END YOUR CODE ***

			// transform the normals (using the inverse transpose of the upper 3x3 block)
			t.norms[0] = (modelToWorld.block<3, 3>(0, 0).inverse().transpose() * n0).normalized();
			t.norms[1] = (modelToWorld.block<3, 3>(0, 0).inverse().transpose() * n1).normalized();
			t.norms[2] = (modelToWorld.block<3, 3>(0, 0).inverse().transpose() * n2).normalized();

			t.texs[0] = mesh.texs[mesh.tFaces[i][0]];
			t.texs[1] = mesh.texs[mesh.tFaces[i][1]];
			t.texs[2] = mesh.texs[mesh.tFaces[i][2]];



			drawTriangle(image, width, height, zBuffer, t, lights, albedoTexture, texWidth, texHeight);
		}
	}
}


int main()
{
	std::string outputFilename = "output.png";

	const int width = 1920, height = 1080;
	const int nChannels = 4;

	// Setting up an image buffer
	// This std::vector has one 8-bit value for each pixel in each row and column of the image, and
	// for each of the 4 channels (red, green, blue and alpha).
	// Remember 8-bit unsigned values can range from 0 to 255.
	std::vector<uint8_t> imageBuffer(height * width * nChannels);
	std::vector<float> zBuffer(height * width);

	// This line sets the image to black initially.
	Color black{ 0,0,0,255 };
	for (int r = 0; r < height; ++r) {
		for (int c = 0; c < width; ++c) {
			setPixel(imageBuffer, c, r, width, height, black);
			zBuffer[r * width + c] = 1.0f;
		}
	}

	// ========== Subtask 3: Camera Matrices ========

	// *** YOUR CODE HERE ***
	// This makes the projection matrix, using the function you implemented. Once the code is working,
	// try changing the FoV!
	Eigen::Matrix4f projection = projectionMatrix(height, width);

	// This matrix rotates the camera, tilting it down, then translates it up to make it look down on the scene.
	// Once your code is working, try changing this to move the camera around!
	Eigen::Matrix4f cameraToWorld = translationMatrix(Eigen::Vector3f(-0.05f, 0.4f, 1.55f)) * rotateXMatrix(0.35) * rotateYMatrix(-0.04);

	// The main important task = set up the worldToCamera and worldToClip matrices here!
	// Set up worldToCamera, based on cameraToWorld above
	Eigen::Matrix4f worldToCamera = cameraToWorld.inverse();
	// Set up worldToClip, using the projection and worldToCamera matrices
	Eigen::Matrix4f worldToClip = projection * worldToCamera;


	// *** END YOUR CODE ***

	std::string bunnyFilename = "../models/stanford_bunny_texmapped.obj";
	std::string roadFilename = "../models/Road/Road.obj";
	std::string sideHillFilename = "../models/SideHill/SideHill.obj";
	std::string CliffHillFilename = "../models/CliffHill/CliffHill.obj";
	std::string GroundLeavesFilename = "../models/FloorLeaves/FloorLeaves.obj";

	std::vector<std::unique_ptr<Light>> lights;
	// I've already added an ambient light for you!
	lights.emplace_back(new AmbientLight(Eigen::Vector3f(0.2f, 0.2f, 0.2f)));

	//lights.emplace_back(new PointLight(Eigen::Vector3f(1.1f, 1.1f, 1.1f), Eigen::Vector3f(0.f, 1.0f, 0.f)));
	//lights.emplace_back(new PointLight(Eigen::Vector3f(1.0f, 1.0f, 1.0f), Eigen::Vector3f(0.f, 0.f, 0.f)));
	lights.emplace_back(new DirectionalLight(Eigen::Vector3f(1.3f, 1.3f, 1.3f), Eigen::Vector3f(0.f, 0.f, 1.0f)));
	//lights.emplace_back(new SpotLight(Eigen::Vector3f(10.0f, 0.0f, 0.0f), Eigen::Vector3f(0.f, 1.f, 0.0f), Eigen::Vector3f(0, -1, 0), M_PI/8));

	Mesh bunnyMesh;
	try {
		bunnyMesh = loadMeshFile(bunnyFilename);
		std::cout << "Successfully loaded model: " << bunnyFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << bunnyFilename << "\nReason: " << e.what() << std::endl;
	}

	Mesh roadMesh;
	try {
		roadMesh = loadMeshFile(roadFilename);
		std::cout << "Successfully loaded model: " << roadFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << roadFilename << "\nReason: " << e.what() << std::endl;
	}
	Mesh sideHillMesh;
	try {
		sideHillMesh = loadMeshFile(sideHillFilename);
		std::cout << "Successfully loaded model: " << sideHillFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << sideHillFilename << "\nReason: " << e.what() << std::endl;
	}
	Mesh CliffHillMesh;
	try {
		CliffHillMesh = loadMeshFile(CliffHillFilename);
		std::cout << "Successfully loaded model: " << CliffHillFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CliffHillFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f bunnyTransform;
	Eigen::Matrix4f roadTransform;
	Eigen::Matrix4f sideHillTransform;
	Eigen::Matrix4f CliffHillTransform;

	std::vector<uint8_t> bunnyTexture;
	unsigned int bunnyTexWidth, bunnyTexHeight;
	lodepng::decode(bunnyTexture, bunnyTexWidth, bunnyTexHeight, "../models/stanford_bunny_albedo.png");

	std::vector<uint8_t> roadTexture;
	unsigned int roadTexWidth, roadTexHeight;
	lodepng::decode(roadTexture, roadTexWidth, roadTexHeight, "../models/Road/RoadTexture.png");

	std::vector<uint8_t> sideHillTexture;
	unsigned int sideHillTexWidth, sideHillTexHeight;
	lodepng::decode(sideHillTexture, sideHillTexWidth, sideHillTexHeight, "../models/SideHill/SideHillTexture.png");

	std::vector<uint8_t> CliffHillTexture;
	unsigned int CliffHillTexWidth, CliffHillTexHeight;
	lodepng::decode(CliffHillTexture, CliffHillTexWidth, CliffHillTexHeight, "../models/CliffHill/CliffHillTexture.png");

	bunnyTransform = translationMatrix(Eigen::Vector3f(-1.0f, -1.0f, 3.f)) * rotateYMatrix(M_PI);
	//drawMesh(imageBuffer, zBuffer, bunnyMesh, bunnyTexture, bunnyTexWidth, bunnyTexHeight, bunnyTransform, worldToClip, lights, width, height);

	roadTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.f, 3.0f));
	drawMesh(imageBuffer, zBuffer, roadMesh, roadTexture, roadTexWidth, roadTexHeight, roadTransform, worldToClip, lights, width, height);

	sideHillTransform = translationMatrix(Eigen::Vector3f(0.01f, 0.f, 3.0f));
	drawMesh(imageBuffer, zBuffer, sideHillMesh, sideHillTexture, sideHillTexWidth, sideHillTexHeight, sideHillTransform, worldToClip, lights, width, height);

	CliffHillTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CliffHillMesh, CliffHillTexture, CliffHillTexWidth, CliffHillTexHeight, CliffHillTransform, worldToClip, lights, width, height);

	std::string AutumnTreesFilename = "../models/AutumnTrees/AutumnTrees.obj";
	Mesh AutumnTreesMesh;
	try {
		AutumnTreesMesh = loadMeshFile(AutumnTreesFilename);
		std::cout << "Successfully loaded model: " << AutumnTreesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << AutumnTreesFilename << "\nReason: " << e.what() << std::endl;
	}
	Eigen::Matrix4f AutumnTreesTransform;
	std::vector<uint8_t> AutumnTreesTexture;
	unsigned int AutumnTreesTexWidth, AutumnTreesTexHeight;
	lodepng::decode(AutumnTreesTexture, AutumnTreesTexWidth, AutumnTreesTexHeight, "../models/AutumnTrees/AutumnTreesTexture.png");
	AutumnTreesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, AutumnTreesMesh, AutumnTreesTexture, AutumnTreesTexWidth, AutumnTreesTexHeight, AutumnTreesTransform, worldToClip, lights, width, height);

	std::string AutumnLeavesFilename = "../models/AutumnLeaves/AutumnLeaves4.obj";
	Mesh AutumnLeavesMesh;
	try {
		AutumnLeavesMesh = loadMeshFile(AutumnLeavesFilename);
		std::cout << "Successfully loaded model: " << AutumnLeavesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << AutumnLeavesFilename << "\nReason: " << e.what() << std::endl;
	}
	Eigen::Matrix4f AutumnLeavesTransform;
	std::vector<uint8_t> AutumnLeavesTexture;
	unsigned int AutumnLeavesTexWidth, AutumnLeavesTexHeight;
	lodepng::decode(AutumnLeavesTexture, AutumnLeavesTexWidth, AutumnLeavesTexHeight, "../models/AutumnLeaves/AutumnLeavesTexture.png");
	AutumnLeavesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, AutumnLeavesMesh, AutumnLeavesTexture, AutumnLeavesTexWidth, AutumnLeavesTexHeight, AutumnLeavesTransform, worldToClip, lights, width, height);

	std::string BackgroundHillFilename = "../models/BackgroundHill/BackgroundHill.obj";
	Mesh BackgroundHillMesh;
	try {
		BackgroundHillMesh = loadMeshFile(BackgroundHillFilename);
		std::cout << "Successfully loaded model: " << BackgroundHillFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << BackgroundHillFilename << "\nReason: " << e.what() << std::endl;
	}
	Eigen::Matrix4f BackgroundHillTransform;
	std::vector<uint8_t> BackgroundHillTexture;
	unsigned int BackgroundHillTexWidth, BackgroundHillTexHeight;
	lodepng::decode(BackgroundHillTexture, BackgroundHillTexWidth, BackgroundHillTexHeight, "../models/BackgroundHill/BackgroundHillTexture.png");
	BackgroundHillTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.f, 3.0f));
	drawMesh(imageBuffer, zBuffer, BackgroundHillMesh, BackgroundHillTexture, BackgroundHillTexWidth, BackgroundHillTexHeight, BackgroundHillTransform, worldToClip, lights, width, height);

	std::string BackgroundLeavesFilename = "../models/BackgroundLeaves/BackgroundLeaves.obj";
	Mesh BackgroundLeavesMesh;
	try {
		BackgroundLeavesMesh = loadMeshFile(BackgroundLeavesFilename);
		std::cout << "Successfully loaded model: " << BackgroundLeavesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << BackgroundLeavesFilename << "\nReason: " << e.what() << std::endl;
	}
	Eigen::Matrix4f BackgroundLeavesTransform;
	std::vector<uint8_t> BackgroundLeavesTexture;
	unsigned int BackgroundLeavesTexWidth, BackgroundLeavesTexHeight;
	lodepng::decode(BackgroundLeavesTexture, BackgroundLeavesTexWidth, BackgroundLeavesTexHeight, "../models/BackgroundLeaves/BackgroundLeavesTexture.png");
	BackgroundLeavesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, BackgroundLeavesMesh, BackgroundLeavesTexture, BackgroundLeavesTexWidth, BackgroundLeavesTexHeight, BackgroundLeavesTransform, worldToClip, lights, width, height);

	std::string BackgroundPostFilename = "../models/BackgroundPost/BackgroundPost.obj";

	Mesh BackgroundPostMesh;
	try {
		BackgroundPostMesh = loadMeshFile(BackgroundPostFilename);
		std::cout << "Successfully loaded model: " << BackgroundPostFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << BackgroundPostFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f BackgroundPostTransform;

	std::vector<uint8_t> BackgroundPostTexture;
	unsigned int BackgroundPostTexWidth, BackgroundPostTexHeight;
	lodepng::decode(BackgroundPostTexture, BackgroundPostTexWidth, BackgroundPostTexHeight, "../models/BackgroundPost/BackgroundPostTexture.png");

	BackgroundPostTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, BackgroundPostMesh, BackgroundPostTexture, BackgroundPostTexWidth, BackgroundPostTexHeight, BackgroundPostTransform, worldToClip, lights, width, height);

	std::string BackgroundRocks_HFilename = "../models/Backgroundrock_H/BackgroundRocks_H.obj";

	Mesh BackgroundRocks_HMesh;
	try {
		BackgroundRocks_HMesh = loadMeshFile(BackgroundRocks_HFilename);
		std::cout << "Successfully loaded model: " << BackgroundRocks_HFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << BackgroundRocks_HFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f BackgroundRocks_HTransform;

	std::vector<uint8_t> BackgroundRocks_HTexture;
	unsigned int BackgroundRocks_HTexWidth, BackgroundRocks_HTexHeight;
	lodepng::decode(BackgroundRocks_HTexture, BackgroundRocks_HTexWidth, BackgroundRocks_HTexHeight, "../models/Backgroundrock_H/BackgroundRocks_HTexture.png");

	BackgroundRocks_HTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, BackgroundRocks_HMesh, BackgroundRocks_HTexture, BackgroundRocks_HTexWidth, BackgroundRocks_HTexHeight, BackgroundRocks_HTransform, worldToClip, lights, width, height);

	std::string BackgroundRocksFilename = "../models/BackgroundRocks/BackgroundRocks.obj";

	Mesh BackgroundRocksMesh;
	try {
		BackgroundRocksMesh = loadMeshFile(BackgroundRocksFilename);
		std::cout << "Successfully loaded model: " << BackgroundRocksFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << BackgroundRocksFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f BackgroundRocksTransform;

	std::vector<uint8_t> BackgroundRocksTexture;
	unsigned int BackgroundRocksTexWidth, BackgroundRocksTexHeight;
	lodepng::decode(BackgroundRocksTexture, BackgroundRocksTexWidth, BackgroundRocksTexHeight, "../models/BackgroundRocks/BackgroundRocksTexture.png");

	BackgroundRocksTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, BackgroundRocksMesh, BackgroundRocksTexture, BackgroundRocksTexWidth, BackgroundRocksTexHeight, BackgroundRocksTransform, worldToClip, lights, width, height);

	std::string CarBodyFilename = "../models/CarBody/CarBody2.obj";

	Mesh CarBodyMesh;
	try {
		CarBodyMesh = loadMeshFile(CarBodyFilename);
		std::cout << "Successfully loaded model: " << CarBodyFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CarBodyFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f  CarBodyTransform;

	std::vector<uint8_t>  CarBodyTexture;
	unsigned int CarBodyTexWidth, CarBodyTexHeight;
	lodepng::decode(CarBodyTexture, CarBodyTexWidth, CarBodyTexHeight, "../models/CarBody/CarBodyTexture.png");

	CarBodyTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CarBodyMesh, CarBodyTexture, CarBodyTexWidth, CarBodyTexHeight, CarBodyTransform, worldToClip, lights, width, height);

	std::string CarTireFilename = "../models/CarTire/CarTire.obj";

	Mesh CarTireMesh;
	try {
		CarTireMesh = loadMeshFile(CarTireFilename);
		std::cout << "Successfully loaded model: " << CarTireFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CarTireFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f  CarTireTransform;

	std::vector<uint8_t> CarTireTexture;
	unsigned int CarTireTexWidth, CarTireTexHeight;
	lodepng::decode(CarTireTexture, CarTireTexWidth, CarTireTexHeight, "../models/CarTire/CarTireTexture.png");

	CarTireTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CarTireMesh, CarTireTexture, CarTireTexWidth, CarTireTexHeight, CarTireTransform, worldToClip, lights, width, height);

	std::string CarBadgeFilename = "../models/CarBadge/CarBadge.obj";

	Mesh  CarBadgeMesh;
	try {
		CarBadgeMesh = loadMeshFile(CarBadgeFilename);
		std::cout << "Successfully loaded model: " << CarBadgeFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CarBadgeFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f   CarBadgeTransform;

	std::vector<uint8_t> CarBadgeTexture;
	unsigned int  CarBadgeTexWidth, CarBadgeTexHeight;
	lodepng::decode(CarBadgeTexture, CarBadgeTexWidth, CarBadgeTexHeight, "../models/CarBadge/CarBadgeTexture.png");

	CarBadgeTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CarBadgeMesh, CarBadgeTexture, CarBadgeTexWidth, CarBadgeTexHeight, CarBadgeTransform, worldToClip, lights, width, height);

	std::string CarRimsFilename = "../models/CarRims/CarRims.obj";

	Mesh  CarRimsMesh;
	try {
		CarRimsMesh = loadMeshFile(CarRimsFilename);
		std::cout << "Successfully loaded model: " << CarRimsFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CarRimsFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f CarRimsTransform;

	std::vector<uint8_t>CarRimsTexture;
	unsigned int  CarRimsTexWidth, CarRimsTexHeight;
	lodepng::decode(CarRimsTexture, CarRimsTexWidth, CarRimsTexHeight, "../models/CarRims/CarRimsTexture.png");

	CarRimsTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CarRimsMesh, CarRimsTexture, CarRimsTexWidth, CarRimsTexHeight, CarRimsTransform, worldToClip, lights, width, height);

	std::string CarMirrorFilename = "../models/CarMirror/CarMirror.obj";

	Mesh  CarMirrorMesh;
	try {
		CarMirrorMesh = loadMeshFile(CarMirrorFilename);
		std::cout << "Successfully loaded model: " << CarMirrorFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CarMirrorFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f CarMirrorTransform;

	std::vector<uint8_t>CarMirrorTexture;
	unsigned int  CarMirrorTexWidth, CarMirrorTexHeight;
	lodepng::decode(CarMirrorTexture, CarMirrorTexWidth, CarMirrorTexHeight, "../models/CarMirror/CarMirrorTexture.png");

	CarMirrorTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CarMirrorMesh, CarMirrorTexture, CarMirrorTexWidth, CarMirrorTexHeight, CarMirrorTransform, worldToClip, lights, width, height);

	std::string CarWindowFilename = "../models/CarWindow/CarWindow.obj";

	Mesh  CarWindowMesh;
	try {
		CarWindowMesh = loadMeshFile(CarWindowFilename);
		std::cout << "Successfully loaded model: " << CarWindowFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CarWindowFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f CarWindowTransform;

	std::vector<uint8_t> CarWindowTexture;
	unsigned int  CarWindowTexWidth, CarWindowTexHeight;
	lodepng::decode(CarWindowTexture, CarWindowTexWidth, CarWindowTexHeight, "../models/CarWindow/CarWindowTexture.png");

	CarWindowTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CarWindowMesh, CarWindowTexture, CarWindowTexWidth, CarWindowTexHeight, CarWindowTransform, worldToClip, lights, width, height);

	std::string CliffGrassFilename = "../models/CliffGrass/CliffGrass.obj";

	Mesh CliffGrassMesh;
	try {
		CliffGrassMesh = loadMeshFile(CliffGrassFilename);
		std::cout << "Successfully loaded model: " << CliffGrassFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CliffGrassFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f CliffGrassTransform;

	std::vector<uint8_t> CliffGrassTexture;
	unsigned int CliffGrassTexWidth, CliffGrassTexHeight;
	lodepng::decode(CliffGrassTexture, CliffGrassTexWidth, CliffGrassTexHeight, "../models/CliffGrass/CliffGrassTexture.png");

	CliffGrassTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CliffGrassMesh, CliffGrassTexture, CliffGrassTexWidth, CliffGrassTexHeight, CliffGrassTransform, worldToClip, lights, width, height);

	std::string CliffPostFilename = "../models/CliffPost/CliffPosts.obj";

	Mesh CliffPostMesh;
	try {
		CliffPostMesh = loadMeshFile(CliffPostFilename);
		std::cout << "Successfully loaded model: " << CliffPostFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CliffPostFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f CliffPostTransform;

	std::vector<uint8_t> CliffPostTexture;
	unsigned int CliffPostTexWidth, CliffPostTexHeight;
	lodepng::decode(CliffPostTexture, CliffPostTexWidth, CliffPostTexHeight, "../models/CliffPost/CliffPostTexture.png");

	CliffPostTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CliffPostMesh, CliffPostTexture, CliffPostTexWidth, CliffPostTexHeight, CliffPostTransform, worldToClip, lights, width, height);

	std::string CliffRocksFilename = "../models/CliffRocks/CliffRocks.obj";

	Mesh CliffRocksMesh;
	try {
		CliffRocksMesh = loadMeshFile(CliffRocksFilename);
		std::cout << "Successfully loaded model: " << CliffRocksFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << CliffRocksFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f CliffRocksTransform;

	std::vector<uint8_t> CliffRocksTexture;
	unsigned int CliffRocksTexWidth, CliffRocksTexHeight;
	lodepng::decode(CliffRocksTexture, CliffRocksTexWidth, CliffRocksTexHeight, "../models/CliffRocks/CliffRocksTexture.png");

	CliffRocksTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, CliffRocksMesh, CliffRocksTexture, CliffRocksTexWidth, CliffRocksTexHeight, CliffRocksTransform, worldToClip, lights, width, height);

	std::string FloorLeavesFilename = "../models/FloorLeaves/FloorLeaves.obj";

	Mesh FloorLeavesMesh;
	try {
		FloorLeavesMesh = loadMeshFile(FloorLeavesFilename);
		std::cout << "Successfully loaded model: " << FloorLeavesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << FloorLeavesFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f FloorLeavesTransform;

	std::vector<uint8_t> FloorLeavesTexture;
	unsigned int FloorLeavesTexWidth, FloorLeavesTexHeight;
	lodepng::decode(FloorLeavesTexture, FloorLeavesTexWidth, FloorLeavesTexHeight, "../models/FloorLeaves/FloorLeavesTexture1.png");

	FloorLeavesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, FloorLeavesMesh, FloorLeavesTexture, FloorLeavesTexWidth, FloorLeavesTexHeight, FloorLeavesTransform, worldToClip, lights, width, height);

	std::string NormalTreesFilename = "../models/NormalTrees/NormalTrees.obj";

	Mesh NormalTreesMesh;
	try {
		NormalTreesMesh = loadMeshFile(NormalTreesFilename);
		std::cout << "Successfully loaded model: " << NormalTreesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << NormalTreesFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f NormalTreesTransform;

	std::vector<uint8_t> NormalTreesTexture;
	unsigned int NormalTreesTexWidth, NormalTreesTexHeight;
	lodepng::decode(NormalTreesTexture, NormalTreesTexWidth, NormalTreesTexHeight, "../models/NormalTrees/NormalTreesTexture.png");

	NormalTreesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, NormalTreesMesh, NormalTreesTexture, NormalTreesTexWidth, NormalTreesTexHeight, NormalTreesTransform, worldToClip, lights, width, height);

	std::string NormalLeavesFilename = "../models/NormalLeaves/NormalLeaves.obj";

	Mesh NormalLeavesMesh;
	try {
		NormalLeavesMesh = loadMeshFile(NormalLeavesFilename);
		std::cout << "Successfully loaded model: " << NormalLeavesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << NormalLeavesFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f NormalLeavesTransform;

	std::vector<uint8_t> NormalLeavesTexture;
	unsigned int NormalLeavesTexWidth, NormalLeavesTexHeight;
	lodepng::decode(NormalLeavesTexture, NormalLeavesTexWidth, NormalLeavesTexHeight, "../models/NormalLeaves/NormalLeavesTexture.png");

	NormalLeavesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, NormalLeavesMesh, NormalLeavesTexture, NormalLeavesTexWidth, NormalLeavesTexHeight, NormalLeavesTransform, worldToClip, lights, width, height);

	std::string SideHill_GrassFilename = "../models/SideHill_Grass/SideHill_Grass.obj";

	Mesh SideHill_GrassMesh;
	try {
		SideHill_GrassMesh = loadMeshFile(SideHill_GrassFilename);
		std::cout << "Successfully loaded model: " << SideHill_GrassFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << SideHill_GrassFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f SideHill_GrassTransform;

	std::vector<uint8_t> SideHill_GrassTexture;
	unsigned int SideHill_GrassTexWidth, SideHill_GrassTexHeight;
	lodepng::decode(SideHill_GrassTexture, SideHill_GrassTexWidth, SideHill_GrassTexHeight, "../models/SideHill_Grass/SideHillGrassTexture.png");

	SideHill_GrassTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.f));
	drawMesh(imageBuffer, zBuffer, SideHill_GrassMesh, SideHill_GrassTexture, SideHill_GrassTexWidth, SideHill_GrassTexHeight, SideHill_GrassTransform, worldToClip, lights, width, height);

	std::string SideHill_LeavesFilename = "../models/SideHill_Leaves/SideHill_Leaves.obj";

	Mesh SideHill_LeavesMesh;
	try {
		SideHill_LeavesMesh = loadMeshFile(SideHill_LeavesFilename);
		std::cout << "Successfully loaded model: " << SideHill_LeavesFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << SideHill_LeavesFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f SideHill_LeavesTransform;

	std::vector<uint8_t> SideHill_LeavesTexture;
	unsigned int SideHill_LeavesTexWidth, SideHill_LeavesTexHeight;
	lodepng::decode(SideHill_LeavesTexture, SideHill_LeavesTexWidth, SideHill_LeavesTexHeight, "../models/SideHill_Leaves/SideHillLeavesTexture.png");

	SideHill_LeavesTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.f));
	drawMesh(imageBuffer, zBuffer, SideHill_LeavesMesh, SideHill_LeavesTexture, SideHill_LeavesTexWidth, SideHill_LeavesTexHeight, SideHill_LeavesTransform, worldToClip, lights, width, height);

	std::string SideHill_RockFilename = "../models/SideHill_Rock/SideHill_Rock.obj";

	Mesh SideHill_RockMesh;
	try {
		SideHill_RockMesh = loadMeshFile(SideHill_RockFilename);
		std::cout << "Successfully loaded model: " << SideHill_RockFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << SideHill_RockFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f SideHill_RockTransform;

	std::vector<uint8_t> SideHill_RockTexture;
	unsigned int SideHill_RockTexWidth, SideHill_RockTexHeight;
	lodepng::decode(SideHill_RockTexture, SideHill_RockTexWidth, SideHill_RockTexHeight, "../models/SideHill_Rock/SideHill_RockTexture.png");

	SideHill_RockTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, SideHill_RockMesh, SideHill_RockTexture, SideHill_RockTexWidth, SideHill_RockTexHeight, SideHill_RockTransform, worldToClip, lights, width, height);

	std::string SignFilename = "../models/Sign/Sign.obj";

	Mesh SignMesh;
	try {
		SignMesh = loadMeshFile(SignFilename);
		std::cout << "Successfully loaded model: " << SignFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << SignFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f SignTransform;

	std::vector<uint8_t> SignTexture;
	unsigned int SignTexWidth, SignTexHeight;
	lodepng::decode(SignTexture, SignTexWidth, SignTexHeight, "../models/Sign/SignTexture.png");

	SignTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, SignMesh, SignTexture, SignTexWidth, SignTexHeight, SignTransform, worldToClip, lights, width, height);

	std::string SignLegsFilename = "../models/SignLegs/SignLegs.obj";

	Mesh SignLegsMesh;
	try {
		SignLegsMesh = loadMeshFile(SignLegsFilename);
		std::cout << "Successfully loaded model: " << SignLegsFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << SignLegsFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f SignLegsTransform;

	std::vector<uint8_t> SignLegsTexture;
	unsigned int SignLegsTexWidth, SignLegsTexHeight;
	lodepng::decode(SignLegsTexture, SignLegsTexWidth, SignLegsTexHeight, "../models/SignLegs/SignLegsTexture.png");

	SignLegsTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, SignLegsMesh, SignLegsTexture, SignLegsTexWidth, SignLegsTexHeight, SignLegsTransform, worldToClip, lights, width, height);

	std::string SkyBoxFilename = "../models/SkyBox/SkyBox.obj";

	Mesh SkyBoxMesh;
	try {
		SkyBoxMesh = loadMeshFile(SkyBoxFilename);
		std::cout << "Successfully loaded model: " << SkyBoxFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << SkyBoxFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f SkyBoxTransform;

	std::vector<uint8_t>SkyBoxTexture;
	unsigned int SkyBoxTexWidth, SkyBoxTexHeight;
	lodepng::decode(SkyBoxTexture, SkyBoxTexWidth, SkyBoxTexHeight, "../models/SkyBox/SkyBoxTexture1.png");

	SkyBoxTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, SkyBoxMesh, SkyBoxTexture, SkyBoxTexWidth, SkyBoxTexHeight, SkyBoxTransform, worldToClip, lights, width, height);

	std::string FlooringFilename = "../models/Flooring/Flooring2.obj";

	Mesh FlooringMesh;
	try {
		FlooringMesh = loadMeshFile(FlooringFilename);
		std::cout << "Successfully loaded model: " << FlooringFilename << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load model: " << FlooringFilename << "\nReason: " << e.what() << std::endl;
	}

	Eigen::Matrix4f FlooringTransform;

	std::vector<uint8_t> FlooringTexture;
	unsigned int FlooringTexWidth, FlooringTexHeight;
	lodepng::decode(FlooringTexture, FlooringTexWidth, FlooringTexHeight, "../models/Flooring/FlooringTexture.png");

	FlooringTransform = translationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f));
	drawMesh(imageBuffer, zBuffer, FlooringMesh, FlooringTexture, FlooringTexWidth, FlooringTexHeight, FlooringTransform, worldToClip, lights, width, height);

	for (const auto& vertex : SkyBoxMesh.verts) {
		Eigen::Vector4f transformedVertex = SkyBoxTransform * Eigen::Vector4f(vertex.x(), vertex.y(), vertex.z(), 1.0f);
		std::cout << "Transformed Vertex: " << transformedVertex.transpose() << std::endl;
	}


	// For debug - draw point lights as colored circles so we can see where they are
	drawPointLights(imageBuffer, width, height, lights);

	for (int y = 0; y < height; ++y) // Loop through every row
	{
		for (int x = 0; x < width / 2; ++x) // loop halfway across each row
		{
			// Calculate pixel IDs on the left side
			int leftIdx = (y * width + x) * nChannels;

			// Calculate pixel IDs on the Right side
			int rightIdx = (y * width + (width - 1 - x)) * nChannels;

			// Swap all color channels between left and right
			for (int ch = 0; ch < nChannels; ++ch)
			{
				std::swap(imageBuffer[leftIdx + ch], imageBuffer[rightIdx + ch]);
			}

			// Do same for the ZBuffer
			std::swap(zBuffer[y * width + x], zBuffer[y * width + (width - 1 - x)]);
		}
	}

	// Save the image to png.
	int errorCode;
	errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
	if (errorCode) { // check the error code, in case an error occurred.
		std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
		return errorCode;
	}

	saveZBufferImage("zBuffer.png", zBuffer, width, height);

	return 0;
}
