#include <Eigen/Dense>
#include <lodepng.h>
#include <json/json.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "BVHNode.hpp"
#include "Triangle.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "PointLight.hpp"
#include "DirectionalLight.hpp"
#include "LambertianShader.hpp"
#include "TexturedLambertianShader.hpp"
#include "TexturedPhongShader.hpp"
#include "PhongShader.hpp"
#include "MirrorShader.hpp"
#include "TexCoordTestShader.hpp"
#include "Model.hpp"
#include <fstream>

/// <summary>
/// Load a JSON config file using the nlohmann library.
/// </summary>
nlohmann::json loadConfig(const std::string& filename)
{
	std::ifstream configStream(filename);
	nlohmann::json config = nlohmann::json::parse(configStream);
	return config;
}

/// <summary>
/// Load an Eigen Vector3f from a config file.
/// Call as for example loadVec3FromConfig(config["myVector3"]);
/// </summary>
Eigen::Vector3f loadVec3FromConfig(const nlohmann::json& config)
{
	return Eigen::Vector3f(config[0], config[1], config[2]);
}

int main(int argc, char* argv[]) {

	// *** Load the config file ***
	auto config = loadConfig("../config/config.json");

	const int pixHeight = config["pixHeight"], pixWidth = config["pixWidth"];
	const int nChannels = 4;

	// *** Set up camera and output image ***
	Camera cam(
		loadVec3FromConfig(config["cameraPos"]),
		loadVec3FromConfig(config["cameraForward"]),
		loadVec3FromConfig(config["cameraUp"]),
		pixWidth, pixHeight,
		config["cameraFov"]);


	std::vector<uint8_t> outImage(pixHeight * pixWidth * nChannels);

	Eigen::Vector3f
		red(1.f, 0.f, 0.f),
		blue(0.f, 0.f, 1.f),
		aqua(0.f, .8f, .8f),
		lavender(178.f / 255.f, 164.f / 255.f, 212.f / 255.f);

	// *** Load shaders and textures ***
	//std::vector<uint8_t> spotTexture;
	//unsigned int width, height;
	//lodepng::decode(spotTexture, width, height, "../models/spot.png");

	std::vector<uint8_t> roadTexture;
	unsigned int roadTexWidth, roadTexHeight;
	lodepng::decode(roadTexture, roadTexWidth, roadTexHeight, "../models/Road/RoadTexture.png");

	std::vector<uint8_t> sideHillTexture;
	unsigned int sideHillTexWidth, sideHillTexHeight;
	lodepng::decode(sideHillTexture, sideHillTexWidth, sideHillTexHeight, "../models/SideHill/SideHillTexture.png");

	std::vector<uint8_t> CliffHillTexture;
	unsigned int CliffHillTexWidth, CliffHillTexHeight;
	lodepng::decode(CliffHillTexture, CliffHillTexWidth, CliffHillTexHeight, "../models/CliffHill/CliffHillTexture.png");

	std::vector<uint8_t> AutumnTreesTexture;
	unsigned int AutumnTreesTexWidth, AutumnTreesTexHeight;
	lodepng::decode(AutumnTreesTexture, AutumnTreesTexWidth, AutumnTreesTexHeight, "../models/AutumnTrees/AutumnTreesTexture.png");

	std::vector<uint8_t> AutumnLeavesTexture;
	unsigned int AutumnLeavesTexWidth, AutumnLeavesTexHeight;
	lodepng::decode(AutumnLeavesTexture, AutumnLeavesTexWidth, AutumnLeavesTexHeight, "../models/AutumnLeaves/AutumnLeavesTexture.png");

	std::vector<uint8_t> BackgroundHillTexture;
	unsigned int BackgroundHillTexWidth, BackgroundHillTexHeight;
	lodepng::decode(BackgroundHillTexture, BackgroundHillTexWidth, BackgroundHillTexHeight, "../models/BackgroundHill/BackgroundHillTexture.png");

	std::vector<uint8_t> BackgroundLeavesTexture;
	unsigned int BackgroundLeavesTexWidth, BackgroundLeavesTexHeight;
	lodepng::decode(BackgroundLeavesTexture, BackgroundLeavesTexWidth, BackgroundLeavesTexHeight, "../models/BackgroundLeaves/BackgroundLeavesTexture.png");

	std::vector<uint8_t> BackgroundPostTexture;
	unsigned int BackgroundPostTexWidth, BackgroundPostTexHeight;
	lodepng::decode(BackgroundPostTexture, BackgroundPostTexWidth, BackgroundPostTexHeight, "../models/BackgroundPost/BackgroundPostTexture.png");

	std::vector<uint8_t> BackgroundRocks_HTexture;
	unsigned int BackgroundRocks_HTexWidth, BackgroundRocks_HTexHeight;
	lodepng::decode(BackgroundRocks_HTexture, BackgroundRocks_HTexWidth, BackgroundRocks_HTexHeight, "../models/Backgroundrock_H/BackgroundRocks_HTexture.png");

	std::vector<uint8_t> BackgroundRocksTexture;
	unsigned int BackgroundRocksTexWidth, BackgroundRocksTexHeight;
	lodepng::decode(BackgroundRocksTexture, BackgroundRocksTexWidth, BackgroundRocksTexHeight, "../models/BackgroundRocks/BackgroundRocksTexture.png");

	std::vector<uint8_t>  CarBodyTexture;
	unsigned int CarBodyTexWidth, CarBodyTexHeight;
	lodepng::decode(CarBodyTexture, CarBodyTexWidth, CarBodyTexHeight, "../models/CarBody/CarBodyTexture.png");

	std::vector<uint8_t> CarTireTexture;
	unsigned int CarTireTexWidth, CarTireTexHeight;
	lodepng::decode(CarTireTexture, CarTireTexWidth, CarTireTexHeight, "../models/CarTire/CarTireTexture.png");

	std::vector<uint8_t> CarBadgeTexture;
	unsigned int  CarBadgeTexWidth, CarBadgeTexHeight;
	lodepng::decode(CarBadgeTexture, CarBadgeTexWidth, CarBadgeTexHeight, "../models/CarBadge/CarBadgeTexture.png");

	std::vector<uint8_t>CarRimsTexture;
	unsigned int  CarRimsTexWidth, CarRimsTexHeight;
	lodepng::decode(CarRimsTexture, CarRimsTexWidth, CarRimsTexHeight, "../models/CarRims/CarRimsTexture.png");

	std::vector<uint8_t>CarMirrorTexture;
	unsigned int  CarMirrorTexWidth, CarMirrorTexHeight;
	lodepng::decode(CarMirrorTexture, CarMirrorTexWidth, CarMirrorTexHeight, "../models/CarMirror/CarMirrorTexture.png");

	std::vector<uint8_t> CarWindowTexture;
	unsigned int  CarWindowTexWidth, CarWindowTexHeight;
	lodepng::decode(CarWindowTexture, CarWindowTexWidth, CarWindowTexHeight, "../models/CarWindow/CarWindowTexture.png");

	std::vector<uint8_t> CliffGrassTexture;
	unsigned int CliffGrassTexWidth, CliffGrassTexHeight;
	lodepng::decode(CliffGrassTexture, CliffGrassTexWidth, CliffGrassTexHeight, "../models/CliffGrass/CliffGrassTexture.png");

	std::vector<uint8_t> CliffPostTexture;
	unsigned int CliffPostTexWidth, CliffPostTexHeight;
	lodepng::decode(CliffPostTexture, CliffPostTexWidth, CliffPostTexHeight, "../models/CliffPost/CliffPostTexture.png");

	std::vector<uint8_t> CliffRocksTexture;
	unsigned int CliffRocksTexWidth, CliffRocksTexHeight;
	lodepng::decode(CliffRocksTexture, CliffRocksTexWidth, CliffRocksTexHeight, "../models/CliffRocks/CliffRocksTexture.png");

	std::vector<uint8_t> FloorLeavesTexture;
	unsigned int FloorLeavesTexWidth, FloorLeavesTexHeight;
	lodepng::decode(FloorLeavesTexture, FloorLeavesTexWidth, FloorLeavesTexHeight, "../models/FloorLeaves/FloorLeavesTexture1.png");

	std::vector<uint8_t> NormalTreesTexture;
	unsigned int NormalTreesTexWidth, NormalTreesTexHeight;
	lodepng::decode(NormalTreesTexture, NormalTreesTexWidth, NormalTreesTexHeight, "../models/NormalTrees/NormalTreesTexture.png");

	std::vector<uint8_t> NormalLeavesTexture;
	unsigned int NormalLeavesTexWidth, NormalLeavesTexHeight;
	lodepng::decode(NormalLeavesTexture, NormalLeavesTexWidth, NormalLeavesTexHeight, "../models/NormalLeaves/NormalLeavesTexture.png");

	std::vector<uint8_t> SideHill_GrassTexture;
	unsigned int SideHill_GrassTexWidth, SideHill_GrassTexHeight;
	lodepng::decode(SideHill_GrassTexture, SideHill_GrassTexWidth, SideHill_GrassTexHeight, "../models/SideHill_Grass/SideHillGrassTexture.png");

	std::vector<uint8_t> SideHill_LeavesTexture;
	unsigned int SideHill_LeavesTexWidth, SideHill_LeavesTexHeight;
	lodepng::decode(SideHill_LeavesTexture, SideHill_LeavesTexWidth, SideHill_LeavesTexHeight, "../models/SideHill_Leaves/SideHillLeavesTexture.png");

	std::vector<uint8_t> SideHill_RockTexture;
	unsigned int SideHill_RockTexWidth, SideHill_RockTexHeight;
	lodepng::decode(SideHill_RockTexture, SideHill_RockTexWidth, SideHill_RockTexHeight, "../models/SideHill_Rock/SideHill_RockTexture.png");

	std::vector<uint8_t> SignTexture;
	unsigned int SignTexWidth, SignTexHeight;
	lodepng::decode(SignTexture, SignTexWidth, SignTexHeight, "../models/Sign/SignTexture.png");

	std::vector<uint8_t> SignLegsTexture;
	unsigned int SignLegsTexWidth, SignLegsTexHeight;
	lodepng::decode(SignLegsTexture, SignLegsTexWidth, SignLegsTexHeight, "../models/SignLegs/SignLegsTexture.png");

	std::vector<uint8_t>SkyBoxTexture;
	unsigned int SkyBoxTexWidth, SkyBoxTexHeight;
	lodepng::decode(SkyBoxTexture, SkyBoxTexWidth, SkyBoxTexHeight, "../models/SkyBox/SkyBoxTexture1.png");

	std::vector<uint8_t> FlooringTexture;
	unsigned int FlooringTexWidth, FlooringTexHeight;
	lodepng::decode(FlooringTexture, FlooringTexWidth, FlooringTexHeight, "../models/Flooring/FlooringTexture.png");

	unsigned error;
	error = lodepng::decode(roadTexture, roadTexWidth, roadTexHeight, "../models/Road/RoadTexture.png");
	if (error) {
		std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
	}

	error = lodepng::decode(sideHillTexture, sideHillTexWidth, sideHillTexHeight, "../models/SideHill/SideHillTexture.png");
	if (error) {
		std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
	}

	error = lodepng::decode(CliffHillTexture, CliffHillTexWidth, CliffHillTexHeight, "../models/CliffHill/CliffHillTexture.png");
	if (error) {
		std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
	}

	error = lodepng::decode(BackgroundHillTexture, BackgroundHillTexWidth, BackgroundHillTexHeight, "../models/BackgroundHill/BackgroundHillTexture.png");
	if (error) {
		std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
	}

	if (roadTexture.empty()) {
		std::cerr << "Road texture is empty!" << std::endl;
	}

	if (sideHillTexture.empty()) {
		std::cerr << "Road texture is empty!" << std::endl;
	}

	if (CliffHillTexture.empty()) {
		std::cerr << "Road texture is empty!" << std::endl;
	}

	if (BackgroundHillTexture.empty()) {
		std::cerr << "Road texture is empty!" << std::endl;
	}

	LambertianShader redLambertianShader(red);
	PhongShader bluePlasticShader(blue, Eigen::Vector3f(1.f, 1.f, 1.f), 100.f);
	LambertianShader aquaLambertianShader(aqua);
	LambertianShader lavenderLambertianShader(lavender);
	/*TexturedPhongShader spotShader(&spotTexture, width, height, 100, Eigen::Vector3f(1.f, 1.f, 1.f));*/
	//TexturedPhongShader roadShader(&roadTexture, width, height, 100, Eigen::Vector3f(1.f, 1.f, 1.f));
	TexturedLambertianShader RoadShader(&roadTexture, roadTexWidth, roadTexHeight, 1.0f);
	TexturedLambertianShader SideHillShader(&sideHillTexture, sideHillTexWidth, sideHillTexHeight, 1.0f);
	TexturedLambertianShader CliffHillShader(&CliffHillTexture, CliffHillTexWidth, CliffHillTexHeight, 1.0f);
	TexturedLambertianShader BackgroundHillShader(&BackgroundHillTexture, BackgroundHillTexWidth, BackgroundHillTexHeight, 1.0f);
	TexturedPhongShader CarBodyShader(&CarBodyTexture, CarBodyTexWidth, CarBodyTexHeight, 100, Eigen::Vector3f(1.f, 1., 1.f));
	TexturedLambertianShader CarTireShader(&CarTireTexture, CarTireTexWidth, CarTireTexHeight, 1.0f);
	TexturedLambertianShader CarBadgeShader(&CarBadgeTexture, CarBadgeTexWidth, CarBadgeTexHeight, 1.0f);
	TexturedPhongShader CarRimsShader(&CarRimsTexture, CarRimsTexWidth, CarRimsTexHeight, 100, Eigen::Vector3f(1.f, 1., 1.f));
	TexturedLambertianShader CarMirrorShader(&CarMirrorTexture, CarMirrorTexWidth, CarMirrorTexHeight, 1.0f);
	TexturedLambertianShader CliffGrassShader(&CliffGrassTexture, CliffGrassTexWidth, CliffGrassTexHeight, 0.0f);
	TexturedLambertianShader CliffPostShader(&CliffPostTexture, CliffPostTexWidth, CliffPostTexHeight, 1.0f);
	TexturedLambertianShader CliffRocksShader(&CliffRocksTexture, CliffRocksTexWidth, CliffRocksTexHeight, 1.0f);
	TexturedLambertianShader AutumnTreesShader(&AutumnTreesTexture, AutumnTreesTexWidth, AutumnTreesTexHeight, 0.5f);
	TexturedLambertianShader AutumnLeavesShader(&AutumnLeavesTexture, AutumnLeavesTexWidth, AutumnLeavesTexHeight, 0.5f);
	TexturedLambertianShader BackgroundLeavesShader(&BackgroundLeavesTexture, BackgroundLeavesTexWidth, BackgroundLeavesTexHeight, 0.0f);
	TexturedLambertianShader BackgroundPostShader(&BackgroundPostTexture, BackgroundPostTexWidth, BackgroundPostTexHeight, 1.0f);
	TexturedLambertianShader Backgroundrock_HShader(&BackgroundRocks_HTexture, BackgroundRocks_HTexWidth, BackgroundRocks_HTexHeight, 1.0f);
	TexturedLambertianShader BackgroundRocksShader(&BackgroundRocksTexture, BackgroundRocksTexWidth, BackgroundRocksTexHeight, 1.0f);
	TexturedLambertianShader FloorLeavesShader(&FloorLeavesTexture, FloorLeavesTexWidth, FloorLeavesTexHeight, 0.0f);
	TexturedLambertianShader NormalTreesShader(&NormalTreesTexture, NormalTreesTexWidth, NormalTreesTexHeight, 0.0f);
	TexturedLambertianShader NormalLeavesShader(&NormalLeavesTexture, NormalLeavesTexWidth, NormalLeavesTexHeight, 0.0f);
	TexturedLambertianShader SideHill_GrassShader(&SideHill_GrassTexture, SideHill_GrassTexWidth, SideHill_GrassTexHeight, 0.0f);
	TexturedLambertianShader SideHill_LeavesShader(&SideHill_LeavesTexture, SideHill_LeavesTexWidth, SideHill_LeavesTexHeight, 0.0f);
	TexturedLambertianShader SideHill_RockShader(&SideHill_RockTexture, SideHill_RockTexWidth, SideHill_RockTexHeight, 1.0f);
	TexturedLambertianShader SignShader(&SignTexture, SignTexWidth, SignTexHeight, 1.0f);
	TexturedLambertianShader SignLegsShader(&SignLegsTexture, SignLegsTexWidth, SignLegsTexHeight, 1.0f);
	TexturedLambertianShader SkyBoxShader(&SkyBoxTexture, SkyBoxTexWidth, SkyBoxTexHeight, 1.0f);
	TexturedLambertianShader FlooringShader(&FlooringTexture, FlooringTexWidth, FlooringTexHeight, 1.0f);

	MirrorShader mirrorShader;
	TexCoordTestShader texCoordTestShader;

	// *** Set up scene ***
	Scene scene;

	// Optional code: here's how to add the spot mesh to the scene, using a BVH
	// Try enabling this and comparing it to the non-BVH version below!
	/*Model spotModel("../models/spot.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(spotModel, &spotShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 5.0f))* rotateX(M_PI) * rotateY(M_PI)));*/

	Model roadModel("../models/Road/Road.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(roadModel, &RoadShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))));

	Model SideHillModel("../models/SideHill/SideHill.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SideHillModel, &SideHillShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))));

	Model CliffHillModel("../models/CliffHill/CliffHill.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CliffHillModel, &CliffHillShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))));

	Model BackgroundHillModel("../models/BackgroundHill/BackgroundHill.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(BackgroundHillModel, &BackgroundHillShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))));

	Model CarBodyModel("../models/CarBody/CarBody2.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CarBodyModel, &CarBodyShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CarTiresModel("../models/CarTire/CarTire.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CarTiresModel, &CarTireShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CarBadgeModel("../models/CarBadge/CarBadge.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CarBadgeModel, &CarBadgeShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CarRimsModel("../models/CarRims/CarRims.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CarRimsModel, &CarRimsShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CarMirrorModel("../models/CarMirror/CarMirror.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CarMirrorModel, &CarMirrorShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CarWindowModel("../models/CarWindow/CarWindow.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CarWindowModel, &mirrorShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CliffGrassModel("../models/CliffGrass/CliffGrass.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CliffGrassModel, &CliffGrassShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CliffPostModel("../models/CliffPost/CliffPosts.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CliffPostModel, &CliffPostShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model CliffRocksModel("../models/CliffRocks/CliffRocks.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(CliffRocksModel, &CliffRocksShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model AutumnTreesModel("../models/AutumnTrees/AutumnTrees.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(AutumnTreesModel, &AutumnTreesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model AutumnLeavesModel("../models/AutumnLeaves/AutumnLeaves.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(AutumnLeavesModel, &AutumnLeavesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model BackgroundLeavesModel("../models/BackgroundLeaves/BackgroundLeaves.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(BackgroundLeavesModel, &BackgroundLeavesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));

	Model BackgroundPostModel("../models/BackgroundPost/BackgroundPost.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(BackgroundPostModel, &BackgroundPostShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f)) * rotateZ(M_PI)));
	
	Model Backgroundrock_HModel("../models/Backgroundrock_H/BackgroundRocks_H.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(Backgroundrock_HModel, &Backgroundrock_HShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model BackgroundRocksModel("../models/BackgroundRocks/BackgroundRocks.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(BackgroundRocksModel, &BackgroundRocksShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model FloorLeavesModel("../models/FloorLeaves/FloorLeaves.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(FloorLeavesModel, &FloorLeavesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model NormalTreesModel("../models/NormalTrees/NormalTrees.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(NormalTreesModel, &NormalTreesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model NormalLeavesModel("../models/NormalLeaves/NormalLeaves.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(NormalLeavesModel, &NormalLeavesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));
	
	Model SideHill_GrassModel("../models/SideHill_Grass/SideHill_Grass.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SideHill_GrassModel, &SideHill_GrassShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model SideHill_LeavesModel("../models/SideHill_Leaves/SideHill_Leaves.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SideHill_LeavesModel, &SideHill_LeavesShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model SideHill_RockModel("../models/SideHill_Rock/SideHill_Rock.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SideHill_RockModel, &SideHill_RockShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model SignModel("../models/Sign/Sign.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SignModel, &SignShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model SignLegsModel("../models/SignLegs/SignLegs.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SignLegsModel, &SignLegsShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 3.0f))* rotateZ(M_PI)));

	Model SkyboxModel("../models/SkyBox/SkyBox.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(SkyboxModel, &SkyBoxShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, 0.0f, 4.0f))* rotateZ(M_PI)));

	Model FlooringModel("../models/Flooring/Flooring.obj");
	scene.renderables.push_back(std::make_shared<BVHNode>(FlooringModel, &FlooringShader, 4, makeTranslationMatrix(Eigen::Vector3f(0.0f, -0.2f, 3.0f))* rotateZ(M_PI)));

	// *** Add lights to scene ***
	Eigen::Vector3f ambientLight(.9f, .9f, .9f);

	std::vector<std::unique_ptr<Light>> lightSources;
	lightSources.push_back(std::make_unique<PointLight>(Eigen::Vector3f(-1.f, 3.f, -1.f), 3.f * Eigen::Vector3f(1.f, 1.f, 1.f)));
	lightSources.push_back(std::make_unique<DirectionalLight>(Eigen::Vector3f(0.f, 0.f, 2.f), .5f * Eigen::Vector3f(2.f, 2.f, 2.f)));

	// *** Render the scene ***

	// Shuffling the scanline order gets better CPU usage between threads
	// when some lines take longer to render than others.
	std::vector<unsigned int> scanlines(pixHeight);
	for (int i = 0; i < pixHeight; ++i) scanlines[i] = i;

	if (config["shuffleScanlines"]) {
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(scanlines.begin(), scanlines.end(), g);
	}

	auto startTime = std::chrono::steady_clock::now();

	Ray ray = cam.getRay(531, 325);
	HitInfo hitInfo;
	scene.intersect(ray, 1e-6f, 1e6f, hitInfo, VISIBLE_BITMASK);
	float x = hitInfo.hitT;


	#pragma omp parallel for
	for (int y = 0; y < pixHeight; ++y) {
		for (int x = 0; x < pixWidth; ++x) {
			Ray ray = cam.getRay(x, scanlines[y]);
			HitInfo hitInfo;

			Eigen::Vector3f finalColor(0, 0, 0);

			if (scene.intersect(ray, 1e-6f, 1e6f, hitInfo, VISIBLE_BITMASK)) {
				Eigen::Vector3f foregroundColor = hitInfo.shader->getColor(
					hitInfo, &scene,
					lightSources, ambientLight,
					0, config["maxBounces"]);

				foregroundColor = foregroundColor.cwiseMin(1.0f);
				float alpha = hitInfo.shader->getAlpha(hitInfo);  // fixed function name

				if (alpha < 1.0f) {
					// Cast background ray from just beyond the surface
					Ray backgroundRay = ray;
					backgroundRay.origin = hitInfo.location + ray.direction * 1e-4f;

					HitInfo bghitInfo;
					Eigen::Vector3f backgroundColor(0.0f, 0.0f, 0.2f); // default background

					if (scene.intersect(backgroundRay, 1e-6f, 1e6f, bghitInfo, VISIBLE_BITMASK)) {
						backgroundColor = bghitInfo.shader->getColor(
							bghitInfo, &scene,
							lightSources, ambientLight,
							0, config["maxBounces"]);
					}

					finalColor = alpha * foregroundColor + (1.0f - alpha) * backgroundColor;
				}
				else {
					finalColor = foregroundColor;
				}

				finalColor = finalColor.cwiseMin(1.0f);

				int line = (pixHeight - scanlines[y]) - 1;
				int flippedX = pixWidth - x - 1;
				int pixelIndex = (flippedX + line * pixWidth) * nChannels;

				outImage[pixelIndex + 0] = static_cast<uint8_t>(finalColor.x() * 255);
				outImage[pixelIndex + 1] = static_cast<uint8_t>(finalColor.y() * 255);
				outImage[pixelIndex + 2] = static_cast<uint8_t>(finalColor.z() * 255);
				outImage[pixelIndex + 3] = static_cast<uint8_t>(alpha * 255);  // optional
			}
			else {
				// background (no intersection)
				int line = (pixHeight - scanlines[y]) - 1;
				int flippedX = pixWidth - x - 1;
				int pixelIndex = (flippedX + line * pixWidth) * nChannels;

				outImage[pixelIndex + 0] = 0;
				outImage[pixelIndex + 1] = 0;
				outImage[pixelIndex + 2] = 50;
				outImage[pixelIndex + 3] = 255;
			}
			if (omp_get_thread_num() == omp_get_num_threads() - 1) {
				std::clog << "\rScanlines remaining: " << (pixHeight - y) << ' ' << std::flush;
			}
		}
	}

	auto renderTime = std::chrono::steady_clock::now() - startTime;

	std::cout << "Render duration " << std::chrono::duration_cast<std::chrono::milliseconds>(renderTime).count() * 1e-3f << " seconds." << std::endl;

	// *** Save the output image ***
	int errorCode;
	errorCode = lodepng::encode(config["outputFilename"], outImage, pixWidth, pixHeight);
	if (errorCode) { // check the error code, in case an error occurred.
		std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
		return errorCode;
	}

	return 0;
}
