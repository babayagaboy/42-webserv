import MethodInspector from "../components/MethodInspector";

function Options() {
  return (
    <MethodInspector
      method="OPTIONS"
      defaultPath="/options"
      description="See which HTTP methods the server allows for a resource."
    />
  );
}

export default Options;
