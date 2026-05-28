for h in pca pcb pcc; do
  echo "=== $h ==="
  ssh mpiu@$h 'file /mirror/PixFarm/func/para_image_mpi; ldd /mirror/PixFarm/func/para_image_mpi | grep "not found" || echo OK'
done

mpiexec \
  --bind-to none \
  --mca btl_tcp_if_include 192.168.133.0/24 \
  --mca oob_tcp_if_include 192.168.133.0/24 \
  --tag-output \
  -n 3 \
  --host pca,pcb,pcc \
  hostname