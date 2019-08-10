#!/bin/bash

tar -vxf backup/rootfs_16a.tar.gz -C ./
mv rootfs rootfs_16a

tar -vxf backup/rootfs_18c.tar.gz -C ./
mv rootfs rootfs_18c

tar -vxf backup/rootfs_18e.tar.gz -C ./
mv rootfs rootfs_18e

tar -vxf backup/rootfs_18e_p1.tar.gz -C ./
mv rootfs rootfs_18e_p1

tar -vxf backup/rootfs_18e_v200.tar.gz -C ./
#mv rootfs_18e_v200 ../

tar -vxf backup/rootfs_m388c2g.tar.gz -C ./
mv rootfs rootfs_18e_m388c2g

tar -vxf backup/rootfs_18e_v200_p3.tar.gz -C ./
#mv rootfs_18e_v200_p3 ../



