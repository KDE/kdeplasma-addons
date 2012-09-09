.pragma library

function setAlphaF(solidColor, alpha) {
    var solid = String(solidColor);
    var alphaHex = Math.round(alpha * 255).toString(16);

    if (alphaHex.length === 1)
        alphaHex = "0" + alphaHex;

    return "#" + alphaHex + solid.slice(1);
}
