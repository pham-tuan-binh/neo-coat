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
  let fileList = document.getElementsByClassName("file-list")[0];
  let animationSetting = document.getElementsByClassName(
    "animation-setting"
  )[0];
  let animationController = document.getElementsByClassName(
    "animation-controller"
  )[0];
  let toggle = document.getElementsByClassName("toggle")[0];
  let snake = document.getElementsByClassName("snake")[0];
  let backFromAnimation = document.getElementsByClassName("back")[1];
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

  // File list function
  function addFile(name, index) {
    let elem = `<div class="file click">
    <div class="title">
      <svg style="width:20px;height:20px" viewBox="0 0 21 21">
        <path fill="currentColor" d="M8.5,13.5L11,16.5L14.5,12L19,18H5M21,19V5C21,3.89 20.1,3 19,3H5A2,2 0 0,0 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19Z" />
      </svg>
      &nbsp;
      ${index}.${name}
    </div>
    <div class="delete extra-small button round-hover click" id="delete_${name}">
      <svg style="width:24px;height:24px" viewBox="0 0 24 24">
        <path fill="currentColor" d="M19,4H15.5L14.5,3H9.5L8.5,4H5V6H19M6,19A2,2 0 0,0 8,21H16A2,2 0 0,0 18,19V7H6V19Z" />
      </svg>
    </div>
  </div>`;
    fileList.innerHTML = fileList.innerHTML.concat(elem);
  }
  // Event listener
  document.querySelectorAll(".click").forEach((item) => {
    item.addEventListener("click", () => {
      playClickEffect(item);
    });
  });

  document.querySelectorAll(".controller-button").forEach((item) => {
    item.addEventListener("click", () => {
      fetch(`/command?id=${item.id.substr(7)}`).catch(() => {
        alert(
          "The snakes have started a revolution. You no longer have control over them."
        );
      });
    });
  });

  document.querySelectorAll(".back").forEach((item) => {
    item.addEventListener("click", () => {
      container.style.display = "";
      animationController.style.display = "none";
      controller.style.display = "none";

      for (let node_number = 0; node_number < WIDTH * HEIGHT; node_number++) {
        let node = document.getElementById(`node_${node_number}`);

        node.style.background = state ? "gray" : "#2b2b2b";
      }
    });
  });

  backFromAnimation.addEventListener("click", () => {
    fetch("/animation?id=1")
      .then(() => {
        container.style.display = "";
        animationController.style.display = "none";
        controller.style.display = "none";

        fileList.innerHTML = "";
      })
      .catch(() => {
        alert("Something went wrong");
      });
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
          animationController.style.display = "none";
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

  animationSetting.addEventListener("click", async () => {
    if (state) {
      await fetch("/animation?id=0")
        .then((res) => {
          container.style.display = "none";
          controller.style.display = "none";
          animationController.style.display = "";
          console.log("play animation");
        })
        .catch(() => {
          alert("Animation play error");
        });

      await fetch("/list")
        .then((res) => {
          return res.text();
        })
        .then((text) => {
          let obj = JSON.parse(text).filter((name) => name.endsWith(".jpg"));
          obj.forEach((name, index) => {
            addFile(name, index);
          });
        })
        .catch(() => {
          alert(
            "Meow Meow. Some cats have eaten the pictures we sent. Reload to receive it again."
          );
        });

      fileList.querySelectorAll(".click").forEach((item) => {
        item.addEventListener("click", () => {
          playClickEffect(item);
        });
      });

      fileList.querySelectorAll(".delete").forEach((item) => {
        item.addEventListener("click", () => {
          fetch(`/delete?path=${item.id.substr(7)}`, {
            method: "DELETE",
          })
            .then(() => {
              item.parentNode.remove();
              console.log("File deleted");
            })
            .catch(() => {
              alert("Something went wrong.");
            });
        });
      });
    } else {
      alert("It's dark in there. Turn on the lights before you come in.");
    }
  });

  hiddenFile.addEventListener("click", (event) => {
    const image = event.target.files;
    const formData = new FormData();
    image.forEach((imageData) => {
      formData.append("image", imageData, imageData.name);
    });
    fetch("/upload", {
      method: "POST",
      body: formData,
      headers: {
        "Content-Type": "multipart/form-data",
      },
    });
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
