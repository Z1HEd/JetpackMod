#pragma once

#include "4dm.h"

using namespace fdm;

class ItemJetpack : public Item {
public:
	enum FlightMode {
		Flight,
		Hover,
		Off
	};
	FlightMode flightMode = Flight;

	float fuelLevel = 0;
	bool isFuelDeadly = false;
	bool isSelectedFuelDeadly = false;
	bool isFlushing = false;

	static MeshRenderer renderer;

	static stl::string switchSound;
	static stl::string flushSound;
	static stl::string flightSound;
	static stl::string fuelSwitchSound;
	inline static const char* voiceGroup = "ambience";

	void handleFlight(Player* player);
	int getSelectedFuelCount(InventoryPlayer& inventory);
	void consumeSelectedFuel(InventoryPlayer& inventory);
	static void rendererInit();

	bool isCompatible(const std::unique_ptr<Item>& other) override;
	stl::string getName() override;
	bool isDeadly() override;
	uint32_t getStackLimit() override;

	std::unique_ptr<Item> clone() override;
	nlohmann::json saveAttributes() override;
	
	void render(const glm::ivec2& pos) override;
	void renderEntity(const m4::Mat5& MV, bool inHand, const glm::vec4& lightDir) override;
	
};