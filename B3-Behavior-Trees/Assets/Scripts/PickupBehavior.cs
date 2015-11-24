using UnityEngine;
using TreeSharpPlus;
using RootMotion.FinalIK;

public class PickupBehavior : MonoBehaviour 
{
    public Transform object1; //Transform of object to be picked up

    public GameObject participant; //Character that is performing behaviors
    private BehaviorMecanim charMec; //Mecanim of participant

    private BehaviorAgent behaviorAgent;

    public bool interrupt; // Can we interrupt an interaction of an effector?

    void Start()
    {
        charMec = participant.GetComponent<BehaviorMecanim>();

        behaviorAgent = new BehaviorAgent(this.BuildTreeRoot());
        BehaviorManager.Instance.Register(behaviorAgent);
        behaviorAgent.StartBehavior();
    }

    protected Node ST_ApproachAndPickUp(Transform target)
    {
        Val<Vector3> position = Val.V( () => target.position); //Convert position to type Val so it can be passed into tree
        Val<string> handAnim = Val.V( () => "PickupRight");
        Val<bool> playAnim = Val.V(() => true);
        return new Sequence(charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z-1)), charMec.Node_BodyAnimation(handAnim, playAnim));
    }

    private Node BuildTreeRoot()
    {
        return
            new DecoratorLoop(
                new SequenceShuffle(
                    this.ST_ApproachAndPickUp(this.object1) ));
    }
}