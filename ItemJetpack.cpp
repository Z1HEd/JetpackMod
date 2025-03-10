#include "ItemJetpack.h"

MeshRenderer ItemJetpack::renderer{};
//TODO: make a properly looping sound for flight
stl::string ItemJetpack::flightSound = "";
stl::string ItemJetpack::flushSound = "";
stl::string ItemJetpack::switchSound = "";
stl::string ItemJetpack::fuelSwitchSound = "";

int ItemJetpack::getSelectedFuelCount(InventoryPlayer& inventory) {
	int count = 0;
	for (int slot = 0;slot < inventory.getSlotCount(); slot++) {
		Item* i = inventory.getSlot(slot)->get();
		if (i != nullptr && i->getName() == (isSelectedFuelDeadly? "Deadly Fuel" : "Biofuel"))
			count += i->count;

	}
	return count;
}

void ItemJetpack::consumeSelectedFuel(InventoryPlayer& inventory) {
	for (int slot = 0;slot < inventory.getSlotCount(); slot++) {
		Item* i = inventory.getSlot(slot)->get();
		if (i != nullptr && i->getName() == (isSelectedFuelDeadly ? "Deadly Fuel" : "Biofuel")) {
			i->count--;
			if (i->count < 1) inventory.getSlot(slot)->reset();
			fuelLevel = 1.0f;
			isFuelDeadly = isSelectedFuelDeadly;
			return;
		}
	}
}


stl::string ItemJetpack::getName() {
	return "Jetpack";
}

void ItemJetpack::handleFlight(Player* player) {
	static float prevFrameYPos = player->pos.y;
	static float posRounded;
	static float timeSinceLastFlightSound = 0;

	static double lastTime = glfwGetTime() - 0.01;
	double curTime = glfwGetTime();
	double dt = curTime - lastTime;
	lastTime = curTime;

	timeSinceLastFlightSound += dt;

	if (isFlushing && fuelLevel > 0.0f) {
		AudioManager::playSound4D(flushSound, "ambience", player->cameraPos, { 0,0,0,0 });
		fuelLevel = 0.0f;
	}

	uint8_t curBlock;
	curBlock = StateGame::instanceObj->world->getBlock(player->currentBlock - glm::ivec4{ 0,1,0,0 });
	prevFrameYPos = player->pos.y;

	if (fuelLevel <= 0.0f && player->touchingGround && !isFlushing) consumeSelectedFuel(player->inventoryAndEquipment);
	if (player->touchingGround) return;
	if (fuelLevel <= 0.0f) return;

	float thrust = isFuelDeadly ? 80 : 70;

	if (flightMode==Flight) {
		if (!player->keys.space) return;
		player->vel.y=std::min(player->vel.y + thrust*(float)dt , isFuelDeadly ? 15.0f : 12.0f);
		fuelLevel -= (float)dt * (isFuelDeadly ? 0.05 : 0.15);
		if (timeSinceLastFlightSound >= 0.5) {
			AudioManager::playSound4D(flightSound, "ambience", player->cameraPos, { 0,0,0,0 });
			timeSinceLastFlightSound = 0;
		}
	}
	else if (flightMode == Hover) {
		if (player->keys.space) {
			player->vel.y = std::min(player->vel.y + thrust * (float)dt, isFuelDeadly ? 10.0f : 8.0f);
			fuelLevel -= (float)dt * (isFuelDeadly ? 0.04 : 0.13);
		}
		else if (player->keys.shift) {
			player->vel.y = std::max(player->vel.y - thrust, isFuelDeadly ? -7.0f : -5.0f);
			fuelLevel -= (float)dt * (isFuelDeadly ? 0.015 : 0.07);
		}
		else {
			player->vel.y = std::max(player->vel.y - thrust, 0.0f);
			fuelLevel -= (float)dt * (isFuelDeadly ? 0.01 : 0.05);

			if (player->vel.y <0.001 && player->vel.y >-0.001)
				player->pos.y = prevFrameYPos;

			if (!curBlock && std::ceil(player->pos.y) - player->pos.y < 0.01) player->pos.y = std::ceil(player->pos.y) + 0.006;
			else if (curBlock && player->pos.y - std::floor(player->pos.y) < 0.01) { player->pos.y = std::floor(player->pos.y); player->touchingGround = true; }
		}
		if (timeSinceLastFlightSound >= 0.5) {
			AudioManager::playSound4D(flightSound, "ambience", player->cameraPos, { 0,0,0,0 });
			timeSinceLastFlightSound = 0;
		}
	}
}

void ItemJetpack::render(const glm::ivec2& pos) {
	TexRenderer& tr = *ItemTool::tr; // or TexRenderer& tr = ItemTool::tr; after 0.3
	const Tex2D* ogTex = tr.texture; // remember the original texture

	tr.texture = ResourceManager::get("assets/Tools.png", true); // set to custom texture
	tr.setClip(0, 0, 36, 36);
	tr.setPos(pos.x, pos.y, 70, 72);
	tr.render();

	tr.texture = ogTex; // return to the original texture

}

void ItemJetpack::renderEntity(const m4::Mat5& MV, bool inHand, const glm::vec4& lightDir) {

	Player* player = &StateGame::instanceObj->player;

	float switchAngle;
	switch (flightMode)
	{
	case ItemJetpack::Flight:
		switchAngle = -glm::pi<float>() / 6;
		break;
	case ItemJetpack::Hover:
		switchAngle = 0;
		break;
	default:
		switchAngle = glm::pi<float>() / 6;
		break;
	}

	glm::vec4 fuelColor;
	if (isFuelDeadly) 
		fuelColor = glm::vec4{ 147.0f / 255.0f,55.0f / 255.0f,118.0f / 255.0f,0.75f };
	else
		fuelColor = glm::vec4{ 82.0f / 255.0f,144.0f / 255.0f,40.0f / 255.0f,0.75f };

	// =========CONTROLLER==========

	m4::Mat5 controllerHandle = MV;
	controllerHandle.translate(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.001f });
	controllerHandle *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 6
		}
	);
	controllerHandle.scale(glm::vec4{ 0.2f,1.f,0.25f,0.25f });
	controllerHandle.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerBase = MV;
	controllerBase.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerBase *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerBase.scale(glm::vec4{ 0.5f,.7f,0.25f,0.25f });
	controllerBase.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerLed = MV;
	controllerLed.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerLed *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerLed.translate(glm::vec4{ 0.2f, 0.30f, 0.125f, 0.0f });
	controllerLed.scale(glm::vec4{ 0.04f,.04f,.04f,.04f });
	controllerLed.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerButton1 = MV;
	controllerButton1.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerButton1 *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerButton1.translate(glm::vec4{0.15f , -0.26f, 0.125f, 0.0f });
	controllerButton1.scale(glm::vec4{ 0.13f,.13f,0.05f,0.13f });
	controllerButton1.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerButton2 = MV;
	controllerButton2.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerButton2 *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerButton2.translate(glm::vec4{ -0.15f , -0.26f, 0.125f, 0.0f });
	controllerButton2.scale(glm::vec4{ 0.13f,.13f,0.05f,0.13f });
	controllerButton2.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerButton3 = MV;
	controllerButton3.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerButton3 *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerButton3.translate(glm::vec4{ -0.17f , 0.26f, 0.125f, 0.0f });
	controllerButton3.scale(glm::vec4{ 0.10f,.10f,0.05f,0.10f });
	controllerButton3.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerSwitchBase = MV;
	controllerSwitchBase.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerSwitchBase *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerSwitchBase.translate(glm::vec4{ 0.0f , -0.3f, 0.125f, 0.0f });
	controllerSwitchBase *= m4::Rotor
	(
		{
			m4::wedge({1, 0, 0, 0}, {0, 1, 0, 0}), // XY
			-glm::pi<float>() / 4 + switchAngle
		}
	);
	controllerSwitchBase.scale(glm::vec4{ 0.05f,.05f,0.05f,0.05f });
	controllerSwitchBase.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerSwitchArrow = MV;
	controllerSwitchArrow.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerSwitchArrow *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3 
		}
	);
	
	controllerSwitchArrow.translate(glm::vec4{ 0.0f , -0.3f, 0.125f, 0.0f });
	controllerSwitchArrow *= m4::Rotor
	(
		{
			m4::wedge({1, 0, 0, 0}, {0, 1, 0, 0}), // XY
			switchAngle
		}
	);
	controllerSwitchArrow.translate(glm::vec4{ 0.0f , 0.05f, 0.0f, 0.0f });
	controllerSwitchArrow.scale(glm::vec4{ 0.03f,.1f,0.05f,0.03f });
	controllerSwitchArrow.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerFuel = MV;
	controllerFuel.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerFuel *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerFuel.translate(glm::vec4{ 0.0f , -0.15f+0.15f*fuelLevel, 0.125f, 0.0f });
	controllerFuel.scale(glm::vec4{ 0.3f,.30*fuelLevel,0.031f,0.07f });
	controllerFuel.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	m4::Mat5 controllerFuelBackground = MV;
	controllerFuelBackground.translate(glm::vec4{ 0.0f, .6f, -0.5f, 0.001f });
	controllerFuelBackground *= m4::Rotor
	(
		{
			m4::wedge({0, 0, 1, 0}, {0, 1, 0, 0}), // ZY
			-glm::pi<float>() / 3
		}
	);
	controllerFuelBackground.translate(glm::vec4{ 0.0f , 0.0f, 0.124f, 0.0f });
	controllerFuelBackground.scale(glm::vec4{ 0.3f,.30f,0.03f,0.07f });
	controllerFuelBackground.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	const Shader* shader = ShaderManager::get("tetSolidColorNormalShader");

	shader->use();

	glUniform4f(glGetUniformLocation(shader->id(), "lightDir"), lightDir.x, lightDir.y, lightDir.z, lightDir.w);
	//IRON COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"),  166.0f / 255.0f, 164.0f / 255.0f, 158.0f / 255.0f, 1);


	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerHandle) / sizeof(float), &controllerHandle[0][0]);
	renderer.render();
	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerBase) / sizeof(float), &controllerBase[0][0]);
	renderer.render();

	// RED COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 1, 0, 0, 1);

	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerLed) / sizeof(float), &controllerLed[0][0]);
	renderer.render();
	
	// SOLENOID COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 59.0f / 255.f, 180.0f / 255.f, 110.0f / 255.f, 1);

	if (!player->keys.space) {
		glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton1) / sizeof(float), &controllerButton1[0][0]);
		renderer.render();
	}
	if (!player->keys.shift) {
		glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton2) / sizeof(float), &controllerButton2[0][0]);
		renderer.render();
	}
	if (!isFlushing) {
		glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton3) / sizeof(float), &controllerButton3[0][0]);
		renderer.render();
	}

	// HIGHLIGHTED SOLENOID COLOR

	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 59.0f / 255.f, 224.0f / 255.f, 110.0f / 255.f, 1);

	if (player->keys.space) {
		glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton1) / sizeof(float), &controllerButton1[0][0]);
		renderer.render();
	}
	if (player->keys.shift) {
		glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton2) / sizeof(float), &controllerButton2[0][0]);
		renderer.render();
	}
	if (isFlushing) {
		glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerButton3) / sizeof(float), &controllerButton3[0][0]);
		renderer.render();
	}

	// GREY COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 100.0f / 255.f, 100.0f / 255.f, 100.0f / 255.f, 1);

	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerSwitchBase) / sizeof(float), &controllerSwitchBase[0][0]);
	renderer.render();
	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerSwitchArrow) / sizeof(float), &controllerSwitchArrow[0][0]);
	renderer.render();

	// FUEL INDICATOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), fuelColor.r, fuelColor.g, fuelColor.b, 1);
	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerFuel) / sizeof(float), &controllerFuel[0][0]);
	renderer.render();
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), fuelColor.r/2, fuelColor.g/2, fuelColor.b/2, 1);
	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(controllerFuelBackground) / sizeof(float), &controllerFuelBackground[0][0]);
	renderer.render();

	// =========JETPACK==========

	m4::Mat5 jetpackBase = m4::Mat5(1);
	jetpackBase.translate(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.001f });
	jetpackBase.translate(player->pos);
	jetpackBase.scale(glm::vec4{ 1.f,100.f,1.f,1.f });
	jetpackBase.translate(glm::vec4{ -0.5f, -0.5f, -0.5f, -0.5f });

	//IRON COLOR
	glUniform4f(glGetUniformLocation(shader->id(), "inColor"), 166.0f / 255.0f, 164.0f / 255.0f, 158.0f / 255.0f, 1);


	glUniform1fv(glGetUniformLocation(shader->id(), "MV"), sizeof(jetpackBase) / sizeof(float), &jetpackBase[0][0]);
	renderer.render();
}

void ItemJetpack::rendererInit() {
	MeshBuilder mesh{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };
	// vertex position attribute
	mesh.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
	mesh.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));
	// per-cell normal attribute
	mesh.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
	mesh.addAttr(GL_FLOAT, 1, sizeof(GLfloat));

	mesh.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

	renderer.setMesh(&mesh);
}

bool ItemJetpack::isDeadly() { return true; }
uint32_t ItemJetpack::getStackLimit() { return 1; }

nlohmann::json ItemJetpack::saveAttributes() {
	return { { "fuelLevel", fuelLevel }, { "isFuelDeadly", isFuelDeadly}, { "isSelectedFuelDeadly", isSelectedFuelDeadly} };
}

std::unique_ptr<Item> ItemJetpack::clone() {
	auto result = std::make_unique<ItemJetpack>();

	result->fuelLevel = fuelLevel;
	result->isFuelDeadly =isFuelDeadly;
	result->isSelectedFuelDeadly = isSelectedFuelDeadly;

	return result;
}

$hook(void, Player, update,World* world, double dt, EntityPlayer* entityPlayer) {
	original(self, world, dt,entityPlayer);
	ItemJetpack* jetpack;
	jetpack = dynamic_cast<ItemJetpack*>(self->hotbar.getSlot(self->hotbar.selectedIndex)->get());
	if (!jetpack) jetpack = dynamic_cast<ItemJetpack*>(self->equipment.getSlot(0)->get());
	if (!jetpack) return;
	jetpack->handleFlight(self);
}

// instantiating jetpack item
$hookStatic(std::unique_ptr<Item>, Item, instantiateItem, const stl::string& itemName, uint32_t count, const stl::string& type, const nlohmann::json& attributes) {

	if (type != "jetpack")
		return original(itemName, count, type, attributes);

	auto result = std::make_unique<ItemJetpack>();

	result->fuelLevel = (float)attributes["fuelLevel"];
	result->isFuelDeadly=(bool)attributes["isFuelDeadly"];
	result->isSelectedFuelDeadly = (bool)attributes["isSelectedFuelDeadly"];
	result->count = count;

	return result;
}