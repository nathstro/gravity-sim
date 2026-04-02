# TO-DO
* These are features which I want to (and hopefully, will) add to this project.

* lighting:
	* ~~emissive and regular bodies~~
	* shadows
	* ~~bloom + other post processing~~
* better controls:
	* FIX CAMERA JUMPING
	* inertia when moving camera
* ability to resize window
* time scaling (~~pause time~~, slo-mo, fast-forward)
* ~~ability to spawn planets~~
* ability to save systems


states:
left click

on left click press:
	if (editing position)
		state = editing velocity
	if (editing velocity)
		keep going
	if (confirmed)
		keep going

on left click release:
	if (editing position)
		???
	if (editing velocity)
		state = confirmed
	if (confirmed)
		state = confirmed

while isLeftMouseButtonDown:
	velocityTarget = pickPointfromPlane
	velocity of body = normalise(target - bodyPosition) * distance(target - bodyPosition)
	processRendering
	renderPath

