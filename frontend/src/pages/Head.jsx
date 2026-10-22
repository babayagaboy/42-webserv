import MethodInspector from "../components/MethodInspector";

function Head() {
  return (
    <MethodInspector
      method="HEAD"
      defaultPath="/head"
      description="Send a HEAD request and inspect the response headers."
    />
  );
}

export default Head;
