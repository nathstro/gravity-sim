# TO-DO
* These are features which I want to (and hopefully, will) add to this project.

* lighting:
	* ~~emissive and regular bodies~~
	* shadows
	* ~~bloom + other post processing~~
* better controls:
	* FIX CAMERA JUMPING
* ~~ability to resize window~~
* ~~time scaling (pause time, slo-mo, fast-forward)~~
* ~~ability to spawn planets~~
* ability to save systems

let:
1.0 unit distance = 10 million km = 10 Gm
	1 Gm = 0.1 unit distance
1.0 unit mass = 1 earth mass
1.0 second = 1 month

save file:
----------
* current timescale
* current G constant
* list of all bodies with:
	unsigned int ID;
	glm::vec3 previousPosition;
	glm::vec3 position;
	glm::vec3 acceleration;
	glm::vec3 colour;

	float mass;
	float radius;
	bool emissive;
	std::string name;
