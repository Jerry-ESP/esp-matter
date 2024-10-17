There are some patches in this folder, you need apply the patch before you build the project.

Pacthes in different folders should apply in different location:

connectedhomeip: should apply in esp-matter/connectedhomeip/connectedhomeip folder

esp-matter: should apply in esp-matter folder

esp-idf: should apply in esp-idf folder

You can use below command to apply the patch:

    git apply 0001-add-bootloader-plus-link.patch