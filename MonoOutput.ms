print scenematerials
for _mats in scenematerials do
(
	for _curmat in _mats do
	(
		print _curmat.opacityMap.monoOutput
		_curmat.opacityMap.monoOutput = 1
	)
)