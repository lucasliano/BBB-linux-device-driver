# Overlay

Para modificar el el device tree vamos a generar un overlay. El mismo estará encargado de sobreescribir la información del device tree original. 

Como nosotros necesitamos generar el platform driver desde el comienzo, vamos a tener que sobreescribir todo el periférico del i2c0. Para ello vamos a utilizar su alias del dts original (am335x-boneblack).

# uEnv

Para poder indicarle a uBoot que debe cargar el .dtbo (es un .dtb pero se agrega la o de Overlay) hace falta modificar el `/boot/uEnv.txt`. Se deberá configurar para aceptar overlays e indicar el path donde se encuentra el overlay a utilizar. En este caso modificamos las líneas:

```dts
###Custom Cape
#dtb_overlay=<file8>.dtbo
```

Por las líneas:

```dts
###Custom Cape
dtb_overlay=/home/debian/bbb-linux-device-driver/device_tree/lucas_overlay.dtbo
```

Y nos aseguramos de tener `enable_uboot_overlays=1`.

