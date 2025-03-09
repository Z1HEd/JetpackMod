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
	static MeshRenderer renderer;
	static std::string flightSound;

	void handleFlight(Player* player);
	int getSelectedFuelCount(InventoryPlayer& inventory);
	void consumeSelectedFuel(InventoryPlayer& inventory);

	std::unique_ptr<Item> clone() override;
	static void rendererInit();
	stl::string getName() override;
	void render(const glm::ivec2& pos) override;
	void renderEntity(const m4::Mat5& MV, bool inHand, const glm::vec4& lightDir) override;
	bool isDeadly() override;
	uint32_t getStackLimit() override;
	nlohmann::json saveAttributes() override;
private:
	InventorySession openInstance;
};