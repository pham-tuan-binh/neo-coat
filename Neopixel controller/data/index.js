// Global const ( Congig here )
const WIDTH = 6;
const HEIGHT = 6;

// Global var
let currentColor = "#800000";
let state = false;

// Global function
function brightenColor(H) {
  // Convert hex to RGB
  let r = 0,
    g = 0,
    b = 0;
  if (H.length == 4) {
    r = "0x" + H[1] + H[1];
    g = "0x" + H[2] + H[2];
    b = "0x" + H[3] + H[3];
  } else if (H.length == 7) {
    r = "0x" + H[1] + H[2];
    g = "0x" + H[3] + H[4];
    b = "0x" + H[5] + H[6];
  }
  // Then to HSL
  r /= 255;
  g /= 255;
  b /= 255;
  let cmin = Math.min(r, g, b),
    cmax = Math.max(r, g, b),
    delta = cmax - cmin,
    h = 0,
    s = 0,
    l = 0;

  if (delta == 0) h = 0;
  else if (cmax == r) h = ((g - b) / delta) % 6;
  else if (cmax == g) h = (b - r) / delta + 2;
  else h = (r - g) / delta + 4;

  h = Math.round(h * 60);

  if (h < 0) h += 360;

  l = (cmax + cmin) / 2;
  s = delta == 0 ? 0 : delta / (1 - Math.abs(2 * l - 1));
  s = +(s * 100).toFixed(1);
  l = +(l * 100).toFixed(1);

  // Brighten color
  l += 15;
  s -= 15;

  return "hsl(" + h + "," + s + "%," + l + "%)";
}

window.onload = () => {
  // Elements
  let container = document.getElementsByClassName("container")[0];
  let matrix = document.getElementsByClassName("neopixel-matrix")[0];
  let controller = document.getElementsByClassName("controller")[0];
  let colorPicker = document.getElementsByClassName("color-picker")[0];
  let fileUploader = document.getElementsByClassName("file-uploader")[0];
  let toggle = document.getElementsByClassName("toggle")[0];
  let snake = document.getElementsByClassName("snake")[0];
  let back = document.getElementsByClassName("back")[0];
  let hiddenColor = document.getElementById("hidden-color");
  let hiddenFile = document.getElementById("image-input");

  // Init matrix
  for (let node_number = 0; node_number < WIDTH * HEIGHT; node_number++) {
    let node = document.createElement("div");

    node.id = `node_${node_number}`;

    node.className = "neopixel-node";

    node.position = {
      x: (node_number % HEIGHT) + 1,
      y: (node_number - (node_number % HEIGHT)) / HEIGHT + 1,
    };
    node.style.backgroundColor = "#2b2b2b";

    matrix.appendChild(node);
  }

  // Nodes event listener
  for (let node_number = 0; node_number < WIDTH * HEIGHT; node_number++) {
    let node = document.getElementById(`node_${node_number}`);

    node.addEventListener(
      "click",
      () => {
        if (state) {
          fetch(
            `/color?x=${node.position.x}&y=${
              node.position.y
            }&color=${currentColor.substr(1)}`
          )
            .then(() => {
              console.log(`${node.id} changed successfully`);
            })
            .then(() => {
              brightenedColor = brightenColor(currentColor);
              node.style.backgroundColor = currentColor;
              node.style.background = `radial-gradient(circle, ${brightenedColor} 10%, ${currentColor} 100%)`;

              playClickEffect(node);
            })
            .catch(() => {
              alert(
                "Due to your poor drawing skill, the program has forbid you from doing this action."
              );
              console.log(err.message);
            });
        } else alert("Your light is off");
      },
      false
    );
  }

  // Click effect
  function playClickEffect(elem) {
    elem.classList.add("pushed");

    setTimeout(() => {
      elem.classList.remove("pushed");
    }, 200);
  }

  // Event listener
  document.querySelectorAll(".click").forEach((item) => {
    item.addEventListener("click", () => {
      fetch(`/command?id=${item.id.substr(7)}`)
        .then(() => {
          playClickEffect(item);
        })
        .catch(() => {
          alert(
            "The snakes have started a revolution. You no longer have control over them."
          );
        });
    });
  });

  back.addEventListener("click", () => {
    container.style.display = "";
    controller.style.display = "none";

    for (let node_number = 0; node_number < WIDTH * HEIGHT; node_number++) {
      let node = document.getElementById(`node_${node_number}`);

      node.style.background = state ? "gray" : "#2b2b2b";
    }
  });

  toggle.addEventListener("click", () => {
    fetch("/toggle")
      .then(() => {
        for (let node_number = 0; node_number < WIDTH * HEIGHT; node_number++) {
          let node = document.getElementById(`node_${node_number}`);

          node.style.background = state ? "#2b2b2b" : "gray";
        }
        toggle.classList.toggle("pushed");
        state = !state;
      })
      .catch((err) => {
        alert(
          "Noooooo. The toggle button won't work unless the wearer is single. "
        );
        console.log(err.message);
      });
  });

  snake.addEventListener("click", () => {
    if (state) {
      fetch("/snake")
        .then(() => {
          container.style.display = "none";
          controller.style.display = "";
        })
        .catch(() => {
          alert(
            "Oh no the snake has escaped and kill the operator !!!. He will be absent for some time. "
          );
          console.log(err.message);
        });
    } else alert("It's dark in there. Turn on the lights before you come in.");
  });

  fileUploader.addEventListener("click", () => {
    hiddenFile.click();
  });

  colorPicker.addEventListener("click", () => {
    hiddenColor.click();
  });

  hiddenColor.addEventListener(
    "change",
    (event) => {
      currentColor = event.target.value;
      colorPicker.style.backgroundColor = currentColor;
    },
    false
  );

  // Made by Pham Tuan Binh. Feel free to mess with it.
};
